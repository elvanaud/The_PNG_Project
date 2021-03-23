#ifndef DEFLATE_H
#define DEFLATE_H

#include "BitStream.h"
#include "HuffmanTree.h"

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
    void processCompressedBlock(BitStream & in);
    void processDuplicatedSequence(BitStream & in, uint16_t length);
    void loadDynamicHuffmanTrees(BitStream& in);
    void loadFixedHuffmanTrees(BitStream & in);
    void processBlock(BitStream &in);
    bool finalBlock = false;

    HuffmanTree<uint16_t> literalTree;
    HuffmanTree<uint8_t> distanceTree;
};

#endif // DEFLATE_H
