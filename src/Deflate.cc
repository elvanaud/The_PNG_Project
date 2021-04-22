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
    vector<int> codeLengthCodes = {};
    for(int i = 0; i < codeLengthAlphabet.size(); i++)
        codeLengthCodes.push_back(0);
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

void Deflate::loadFixedHuffmanTrees(BitStream & in)
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

uint16_t Deflate::parseTable(BitStream & in, uint16_t val, vector<uint16_t> const &threshold, vector<uint16_t> const &extrabit,vector<uint16_t> const &minValue)
{
    int i = threshold.size()-1;
    for(; i>=0; i--)
    {
        if(val >= threshold[i])
            break;
    }

    uint16_t diff = val-threshold[i];
    uint16_t step = 1<<extrabit[i];
    uint16_t base = diff*step+minValue[i];
    return base+in.read(extrabit[i]);
}

uint16_t Deflate::computeDistance(BitStream & in, uint16_t dist)
{
    vector<uint16_t> threshold  = {0, 4, 6, 8, 10, 12, 14,16,18,20,22,24,26,28};
    vector<uint16_t> extrabit = {0,1,2,3,4,5,6,7,8,9,10,11,12,13};
    vector<uint16_t> minValue = {1,5,9,17,33,65,129,257,513,1025,2049,4097,8193,16385};//1+power of 2

    return parseTable(in,dist,threshold,extrabit,minValue);
}

uint16_t Deflate::computeLength(BitStream & in, uint16_t len)
{
    vector<uint16_t> threshold  = {257, 265, 269, 273, 277, 281, 285};
    vector<uint16_t> extrabit = {0,1,2,3,4,5,0};
    vector<uint16_t> minValue = {3,11,19,35,67,131,255};//3+power of 2

    return parseTable(in,len,threshold,extrabit,minValue);
}

void Deflate::processDuplicatedSequence(BitStream & in, uint16_t len)
{
    if(len == 256) return;
    uint16_t length = computeLength(in,len);
    uint16_t distance = computeDistance(in,distanceTree.readNext(in));

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
        loadFixedHuffmanTrees(in);
    case 2:
        if(blockType == 2)//dynamic huffman
        {
            loadDynamicHuffmanTrees(in);
        }
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
