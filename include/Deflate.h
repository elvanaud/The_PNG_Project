#ifndef DEFLATE_H
#define DEFLATE_H

#include "BitStream.h"

class Deflate
{
public:
    Deflate();
    //Deflate(BitStream &pst);
    void compress();
    BitStream& uncompress(BitStream &in);

private:
    //BitStream & in;
    BitStream out;
    void processUncompressedBlock(BitStream &in);
    void processBlock(BitStream &in);
    bool finalBlock = false;
};

#endif // DEFLATE_H
