#include "HuffmanTree.h"
#include <algorithm>

template<class SymbolType>
HuffmanTree<SymbolType>::HuffmanTree()
{
    //ctor
}

template<class SymbolType>
bool HuffmanTree<SymbolType>::decode(int bit,SymbolType* decodedSymbol)
{
    bit &= 1;
    int nbStates = states.size();
    if(nbStates==0) return false;

    currentState = states[currentState][bit];
    if(currentState >= nbStates)
    {
        *decodedSymbol = huffCodes[currentState-nbStates].symbol;
        currentState = 0;
        return true;
    }
    return false;
}

template<class SymbolType>
vector<int> HuffmanTree<SymbolType>::computeStartCodes(vector<int> const & codeLengths, int maxLength)
{
    int nbCodes = codeLengths.size();
    if(nbCodes==0) throw "Huffman error: empty code lengths";
    vector<int> lengthCount;

    for(int i = 0; i <= maxLength; i++)
        lengthCount.push_back(0);
    vector<int> startCode(lengthCount);

    for(int length : codeLengths)
    {
        if(length>maxLength) throw "Huffman error: max length specified isn't max";
        lengthCount[length]++;
    }

    CodeType lastCode = 0;
    lengthCount[0] = 0;
    for(int i = 1; i<=maxLength; i++)
    {
        lastCode = (lengthCount[i-1]+lastCode)<<1;
        startCode[i] = lastCode;
    }

    return startCode;
}

template<class SymbolType>
void HuffmanTree<SymbolType>::assignCode(int len, vector<int> & startCodes, SymbolType sym)
{
    if(len != 0)
    {
        HuffmanCode code;
        code.code = startCodes[len];
        code.symbol = sym;
        code.length = len;

        code.parentState = addStates(code);
        if(indexableSymbols) huffCodes[code.symbol] = code;
        else huffCodes.push_back(code);
        startCodes[len]++;
    }
}


template<class SymbolType>
void HuffmanTree<SymbolType>::connectStatesToSymbols()
{
    int nbStates = states.size();
    for(unsigned int i = 0; i < huffCodes.size(); i++)
    {
        if(huffCodes[i].parentState >= 0) //throw "Huffman error: parent state is -1";
            states[huffCodes[i].parentState][huffCodes[i].code&1] = i+nbStates;
    }
}

template<class SymbolType>
void HuffmanTree<SymbolType>::preloadSymbolTable(unsigned int alphabetSize)
{
    if(indexableSymbols)
    {
        HuffmanCode emptyCode;
        for(unsigned int i = 0; i < alphabetSize; i++)
            huffCodes.push_back(emptyCode);
    }
}

template<class SymbolType>
void HuffmanTree<SymbolType>::loadFromCodeLength(vector<int> &codeLengths)
{
    loadFromCodeLength(codeLengths, *std::max_element(codeLengths.begin(),codeLengths.end()));
}

template<class SymbolType>
void HuffmanTree<SymbolType>::loadFromCodeLength(vector<int> codeLengths, int maxLength)
{
    indexableSymbols = true;
    vector<int> startCodes = computeStartCodes(codeLengths,maxLength);
    preloadSymbolTable(codeLengths.size());
    for(unsigned int i = 0; i < codeLengths.size(); i++)
    {
        assignCode(codeLengths[i],startCodes,i);
    }

    connectStatesToSymbols();
}

template<class SymbolType>
void HuffmanTree<SymbolType>::loadFromCodeLength(vector<SymbolType> alphabet, bool idxSym, vector<int> codeLengths)
{
    loadFromCodeLength(alphabet, idxSym, codeLengths, *std::max_element(codeLengths.begin(),codeLengths.end()));
}

template<class SymbolType>
void HuffmanTree<SymbolType>::loadFromCodeLength(vector<SymbolType> alphabet, bool idxSym, vector<int> codeLengths, int maxLength)
{
    indexableSymbols = idxSym;
    vector<int> startCodes = computeStartCodes(codeLengths,maxLength);

    preloadSymbolTable(alphabet.size());
    for(unsigned int i = 0; i < codeLengths.size(); i++)
    {
        assignCode(codeLengths[i],startCodes,alphabet[i]);
    }

    connectStatesToSymbols();
}

template<class SymbolType>
int HuffmanTree<SymbolType>::addStates(HuffmanCode code)
{
    unsigned int current = 0;
    int offset = code.length-1;
    bool lastBit = false;

    for(; offset>=0; offset--)
    {
        if(offset == 0)
            lastBit = true;

        if(current == states.size())
        {
            states.push_back(array<int,2> {-1,-1});
        }

        int bit = (code.code>>offset)&1;
        if(!lastBit)
        {
            if(states[current][bit] < 0)
                current = states[current][bit] = states.size();
            else
                current = states[current][bit];
        }
    }
    return current;
}

template<class SymbolType>
SymbolType HuffmanTree<SymbolType>::readNext(BitStream & in)
{
    SymbolType sym;
    while(!decode(in.read(1),&sym));
    return sym;
}

template<class SymbolType>
void HuffmanTree<SymbolType>::write(BitStream & out, SymbolType s)
{
    //The code has to be written in reverse order !
    auto reverseWrite = [&](HuffmanCode & code)
    {
        int offset = code.length-1;
        for(;offset>=0;offset--)
        {
            out.write((code.code>>offset)&1,1);
        }
    };
    if(indexableSymbols)
    {
        reverseWrite(huffCodes[s]);
    }
    else
    {
        for(HuffmanCode code : huffCodes)
        {
            if(code.symbol == s)
            {
                reverseWrite(code);
                break;
            }
        }
    }
}
