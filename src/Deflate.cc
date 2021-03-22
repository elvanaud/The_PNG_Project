#include "Deflate.h"
#include <array>
using std::array;
#include "HuffmanTree.h"

Deflate::Deflate()
{

}


/*Deflate::Deflate(BitStream &pst):in(pst)
{
    //ctor
}*/

void Deflate::processUncompressedBlock(BitStream &in)
{
    in.skipToFullByte();
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
    //return out.getData();//something like that ? => could return the stream itself or a ref to the vector (which can grow as we output data)
}

void Deflate::loadDynamicHuffmanTree(BitStream& in)
{
    int HLIT = in.read(5)+257;
    int HDIST = in.read(5)+1;
    int HCLEN = in.read(4)+4;

    vector<int> codeLengthAlphabet = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
    /*array<int,19>*/vector<int> codeLengthCodes = {};
    for(int i = 0;i<HCLEN;i++)
    {
        //codeLengthCodes[codeLengthAlphabet[i]] = in.read(3);
        //codeLengthCodes[i] = in.read(3);
        codeLengthCodes.push_back(in.read(3));
    }
    HuffmanTree<int> predecodeTree;
    //predecodeTree.loadFromCodeLength(codeLengthAlphabet,vector<int>(codeLengthCodes.begin(),codeLengthCodes.end()),7);
    predecodeTree.loadFromCodeLength(codeLengthAlphabet,codeLengthCodes,7);

    vector<int> compressedLengths;
    for(int i = 0; i < HDIST+HLIT; i++)
    {
        compressedLengths.push_back(predecodeTree.readNext(in));
    }
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
