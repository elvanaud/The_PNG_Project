#ifndef DEFLATE_H
#define DEFLATE_H

#include "BitStream.h"

class Deflate
{
public:
    Deflate();
    //Deflate(BitStream &pst);
    void compress();
    BitStream& uncompress(BitStream &in);//todo: uncompress block by block (as a stream ?)

private:
    //BitStream & in;
    BitStream out;
    void processUncompressedBlock(BitStream &in);
    void loadDynamicHuffmanTree(BitStream& in);
    void processBlock(BitStream &in);
    bool finalBlock = false;
};

#endif // DEFLATE_H
