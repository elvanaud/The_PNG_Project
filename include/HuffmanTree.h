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
    //Used in deflate, assumes lexicographical order of the codes
    //TODO: create a structure to represent the alphabet, it will include indexableSymbols, and a lambda given by the user to map the symbol to the index
    //Implicit alphabet:
    void loadFromCodeLength(vector<int> codeLengths, int maxLength);
    void loadFromCodeLength(vector<int> codeLengths);
    //Explicit alphabet:
    void loadFromCodeLength(vector<SymbolType> alphabet,bool indexableSymbols, vector<int> codeLengths);
    void loadFromCodeLength(vector<SymbolType> alphabet,bool indexableSymbols, vector<int> codeLengths, int maxLength);
    SymbolType readNext(BitStream & in);
    void write(BitStream & in, SymbolType s);
private:
    bool indexableSymbols = true;
    struct HuffmanCode
    {
        SymbolType symbol;
        unsigned int length = 0;
        CodeType code = 0;

        int parentState = -1;
    };
    vector<HuffmanCode> huffCodes; //for some reason gdb crashes when this is named "symbols"
    //vector<HuffmanCode>symbols2;
    vector<array<int,2>> states;

    int currentState = 0;

    //Helper methods to generate the huffman codes from the alphabet and code lengths
    vector<int> computeStartCodes(vector<int> const & codeLengths, int maxLength);
    void assignCode(int len, vector<int> & startCode, SymbolType sym);
    int addStates(HuffmanCode code);
    void preloadSymbolTable(unsigned int alphabetSize);
    void connectStatesToSymbols();
};

#include "HuffmanTree.tcc"

#endif // HUFFMANTREE_H
