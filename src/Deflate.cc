#include "Deflate.h"
#include <iostream>
#include <cassert>
using std::cout;
using std::endl;

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
            //loadDynamicHuffmanTree(in);
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
