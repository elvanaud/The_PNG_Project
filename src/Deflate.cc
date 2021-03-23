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

void Deflate::loadDynamicHuffmanTree(BitStream& in)
{
    unsigned int HLIT = in.read(5)+257;
    unsigned int HDIST = in.read(5)+1;
    unsigned int HCLEN = in.read(4)+4;

    vector<int> codeLengthAlphabet = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
    vector<int> codeLengthCodes = {};
    for(unsigned int i = 0;i<HCLEN;i++)
    {
        codeLengthCodes.push_back(in.read(3));
    }
    HuffmanTree<int> predecodeTree;
    predecodeTree.loadFromCodeLength(codeLengthAlphabet,true,codeLengthCodes,7);

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
    case 2:
        if(blockType == 2)//dynamic huffman
        {
            loadDynamicHuffmanTree(in);
        }
        //processCompressedBlock(in);
        break;
    case 3:
        throw "Deflate Error: use of reserved type block - Aborting";
        return;
    default:
        cout << "Deflate/DataStream error: impossible value found: "<<blockType<<endl;
        return;
    }
}
