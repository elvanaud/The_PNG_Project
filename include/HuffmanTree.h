#ifndef HUFFMANTREE_H
#define HUFFMANTREE_H

#include "Common.h"
#include <array>
using std::array;

//using SymbolType = uint16_t;
using CodeType = uint32_t;
template<class SymbolType>
class HuffmanTree
{
public:
    HuffmanTree();
    bool decode(int bit,SymbolType* decodedSymbol);
    //Used in deflate, assumes lexicographical order
    void loadFromCodeLength(vector<SymbolType> alphabet,vector<int> codeLengths, int maxLength);
    //initStream(BitStream&); SymbolType nextCode();
    SymbolType readNext(BitStream & in);
    void write(BitStream & in, SymbolType s);
private:
    //CodeType currentCode = 0;
    //unsigned int bitLength = 0;
    struct HuffmanCode
    {
        SymbolType symbol;
        unsigned int length = 0;
        CodeType code = 0;

        int parentState = -1;
    };
    vector<HuffmanCode> symbols;//previously named "codes"
    vector<array<int,2>> states;

    int currentState = 0;

    int addStates(HuffmanCode code);
};

#include "HuffmanTree.tcc"

#endif // HUFFMANTREE_H
