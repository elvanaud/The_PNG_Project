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
    uint16_t parseTable(BitStream & in, uint16_t val, vector<uint16_t> const &threshold, vector<uint16_t> const &extrabit,vector<uint16_t> const &minValue);
    uint16_t computeDistance(BitStream & in, uint16_t dist);
    uint16_t computeLength(BitStream & in, uint16_t len);
    void processDuplicatedSequence(BitStream & in, uint16_t length);
    void loadDynamicHuffmanTrees(BitStream& in);
    void loadFixedHuffmanTrees(BitStream & in);
    void processBlock(BitStream &in);
    bool finalBlock = false;

    HuffmanTree<uint16_t> literalTree;
    HuffmanTree<uint8_t> distanceTree;
};

#endif // DEFLATE_H
