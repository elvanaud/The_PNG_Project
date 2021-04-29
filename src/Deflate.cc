#include "Deflate.h"
#include <array>
using std::array;
#include "HuffmanTree.h"

Deflate::Deflate()
{

}

void Deflate::processUncompressedBlock(BitStream &in)
{
    in.skipToFullByte(BitStream::ReadCursor);
    uint16_t len = in.read16(16);
    uint16_t nlen = ~in.read16(16);

    if(len != nlen) throw "[BitStream] Incoherent header for Uncompressed block: len and nlen don't match";
    for(; len > 0; --len)
    {
        out.write(in.read(8),8);
    }
}

BitStream& Deflate::uncompress(BitStream &in)
{
    while(!finalBlock)
    {
        processBlock(in);
    }
    return out;
}

void Deflate::loadDynamicHuffmanTrees(BitStream& in)
{
    unsigned int HLIT = in.read(5)+257;
    unsigned int HDIST = in.read(5)+1;
    unsigned int HCLEN = in.read(4)+4;

    vector<int> codeLengthAlphabet = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
    vector<int> codeLengthCodes(codeLengthAlphabet.size(),0);
    
    for(unsigned int i = 0;i<HCLEN;i++)
    {
        //codeLengthCodes.push_back(in.read(3));
        codeLengthCodes[codeLengthAlphabet[i]] = in.read(3);
    }
    HuffmanTree<int> predecodeTree;
    //predecodeTree.loadFromCodeLength(codeLengthAlphabet,true,codeLengthCodes,7);
    predecodeTree.loadFromCodeLength(codeLengthCodes);

    vector<int> globalCodeLengths;
    int lastLength = 0;

    for(unsigned int totalCodeLength = 0; totalCodeLength < HDIST+HLIT;)
    {
        int v = predecodeTree.readNext(in);
        if(v < 16)
        {
            lastLength = v;
            totalCodeLength++;
            globalCodeLengths.push_back(v);
        }
        else if(v == 16)
        {
            int repeatLast = in.read(2)+3;
            totalCodeLength += repeatLast;
            for(;repeatLast>0;repeatLast--)
                globalCodeLengths.push_back(lastLength);
        }
        else
        {
            int repeatZero = 0;
            if(v==17)
                repeatZero = in.read(3)+3;
            else
                repeatZero = in.read(7)+11;
            lastLength = 0;
            totalCodeLength += repeatZero;
            for(;repeatZero>0;repeatZero--)
                globalCodeLengths.push_back(0);
        }
    }
    if(globalCodeLengths.size() != HLIT+HDIST) throw "dynamic huffman error: hdit and hlit don't match the number of code length read";

    vector<int> distLengths;
    for(unsigned int i = HLIT; i < globalCodeLengths.size(); i++)
        distLengths.push_back(globalCodeLengths[i]);
    globalCodeLengths.resize(HLIT);//becomes literal/length lengths

    literalTree.loadFromCodeLength(globalCodeLengths);
    distanceTree.loadFromCodeLength(distLengths);
}

void Deflate::loadFixedHuffmanTrees()
{
    vector<int> literalCodeLength;
    for(int i = 0; i <= 143; i++)
        literalCodeLength.push_back(8);
    for(int i = 144; i <= 255; i++)
        literalCodeLength.push_back(9);
    for(int i = 256; i <= 279; i++)
        literalCodeLength.push_back(7);
    for(int i = 280; i <= 287; i++)
        literalCodeLength.push_back(8);

    vector<int> distCodeLength;
    for(int i = 0; i <= 31; i++)
        distCodeLength.push_back(5);

    literalTree.loadFromCodeLength(literalCodeLength,9);
    distanceTree.loadFromCodeLength(distCodeLength,5);
}
Deflate::SelectedParse Deflate::tableParse(uint16_t val, vector<uint16_t> const &thresholds, vector<uint16_t> const &extrabits,vector<uint16_t> const &minValues)
{
    int i = thresholds.size()-1;
    for(; i>=0; i--)
    {
        if(val >= thresholds[i])
            break;
    }

    //uint16_t diff = val-threshold[i];
    //uint16_t step = 1<<extrabit[i];
    return SelectedParse{  
                    threshold: thresholds[i],
                    extrabit: extrabits[i],
                    minValue: minValues[i],
                    diff: val-thresholds[i],
                    step: 1<<extrabits[i]};
}
uint16_t Deflate::tableDecode(BitStream & in, ParseTables const & table, uint16_t val)//, vector<uint16_t> const &threshold, vector<uint16_t> const &extrabit,vector<uint16_t> const &minValue)
{
    SelectedParse parsed = tableParse(val, table.threshold, table.extrabit, table.minValue);

    uint16_t base = parsed.diff*parsed.step+parsed.minValue;
    return base+in.read(parsed.extrabit);
}

Deflate::SelectedParse Deflate::tableEncode(BitStream & out, ParseTables const & table, uint16_t val)
{
    SelectedParse parsed = tableParse(val, table.minValue, table.extrabit, table.threshold);//careful: we need to reverse the threshold and minValue(TODO: rename those fields)

    int level = parsed.diff / parsed.step;
    int extra = parsed.diff % parsed.step;
    int code = parsed.minValue + level;

    parsed.extra = extra;
    parsed.code = code;
    //distanceTree.write(out,code);
    //literalTree.write(out,code);
    //out.write(extra,parsed.extrabit);
    return parsed;
}

uint16_t Deflate::computeDistance(BitStream & in, uint16_t dist)
{
    return tableDecode(in,distTable,dist);
}

uint16_t Deflate::computeLength(BitStream & in, uint16_t len)
{
    return tableDecode(in,lengthTable,len);
}

void Deflate::processDuplicatedSequence(BitStream & in, uint16_t len)
{
    if(len == 256) return;
    uint16_t length = computeLength(in,len); //tableDecode(in,lengthTable,len);//
    uint16_t distance = computeDistance(in,distanceTree.readNext(in)); //tableDecode(in,distTable,distanceTree.readNext(in));//

    vector<uint8_t> const & buffer(out.getBufferRef());
    int initialEndOfBuffer = buffer.size();
    int startIndex = initialEndOfBuffer-distance;
    if(startIndex < 0) throw "Deflate Error: A duplicated sequence can't reference data before the beginning of the output stream";

    int currentIndex = startIndex;
    for(;length > 0; length--)
    {
        out.write(buffer[currentIndex],8);
        currentIndex++;
        if(currentIndex>=initialEndOfBuffer)
            currentIndex = startIndex;
    }
}

void Deflate::processCompressedBlock(BitStream & in)
{
    uint16_t litLength;
    do
    {
        litLength = literalTree.readNext(in);
        if(litLength < 256)
            out.write(litLength,8);
        else
            processDuplicatedSequence(in,litLength);
    }while(litLength != 256);
}

void Deflate::processBlock(BitStream &in)
{
    finalBlock = in.read(1);
    int blockType = in.read(2);
    switch(blockType)
    {
    case 0: //Uncompressed block
        processUncompressedBlock(in);
        break;
    case 1:
        loadFixedHuffmanTrees();
        processCompressedBlock(in);
        break;
    case 2: //dynamic huffman
        //loadDynamicHuffmanTrees(in);
        processCompressedBlock(in);
        break;
    case 3:
        throw "Deflate Error: use of reserved type block - Aborting";
        return;
    default:
        cout << "Deflate/DataStream error: impossible value found: "<<blockType<<endl;
        return;
    }
}
