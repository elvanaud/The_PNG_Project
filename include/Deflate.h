#ifndef DEFLATE_H
#define DEFLATE_H

#include "Common.h"
#include "BitStream.h"
#include "HuffmanTree.h"

class Deflate
{
public:
    Deflate();
    //Deflate(BitStream &pst);
    vector<uint8_t> compress(vector<uint8_t> &data);//BitStream &in);
    BitStream& uncompress(BitStream &in);//todo: uncompress block by block (as a stream ?)

private:
    //BitStream & in;
    BitStream out;
    void processUncompressedBlock(BitStream &in);
    void processCompressedBlock(BitStream & in);
    
    struct ParseTables{
        vector<uint16_t> threshold;
        vector<uint16_t> extrabit; 
        vector<uint16_t> minValue; 
    };
    struct SelectedParse{
        int threshold,extrabit,minValue; //all those values were uint16_t
        int diff,step;
        int extra = -1,code=-1;
    };
    SelectedParse tableParse(uint16_t val, vector<uint16_t> const &thresholds, vector<uint16_t> const &extrabits,vector<uint16_t> const &minValues);
    uint16_t tableDecode(BitStream & in, ParseTables const & table, uint16_t val);
    SelectedParse tableEncode(BitStream & in, ParseTables const & table, uint16_t val);
    ParseTables lengthTable{threshold:{257,265,269,273,277,281,285},
                            extrabit: {0,1,2,3,4,5,0},
                            minValue: {3,11,19,35,67,131,255}};//3+power of 2
    ParseTables distTable{  threshold:{0,4,6,8,10,12,14,16,18,20,22,24,26,28},
                            extrabit: {0,1,2,3,4,5,6,7,8,9,10,11,12,13},
                            minValue: {1,5,9,17,33,65,129,257,513,1025,2049,4097,8193,16385}};//1+power of 2
    uint16_t computeDistance(BitStream & in, uint16_t dist);
    uint16_t computeLength(BitStream & in, uint16_t len);
    void processDuplicatedSequence(BitStream & in, uint16_t length);
    void loadDynamicHuffmanTrees(BitStream& in);
    void loadFixedHuffmanTrees();
    void processBlock(BitStream &in);
    bool finalBlock = false;

    void generateDynamicTrees(vector<int> & litFreq, HuffmanTree<uint16_t> & binTree);//,  vector<int> & distFreq);
    void generateDynamicHeader(vector<int> & codeLengths);

    HuffmanTree<uint16_t> literalTree;
    HuffmanTree<uint16_t> distanceTree;//previously was uint8 but I changed it to make it work as a parameter without templates
};

#endif // DEFLATE_H
