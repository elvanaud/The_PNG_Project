#include "HuffmanTree.h"

template<class SymbolType>
HuffmanTree<SymbolType>::HuffmanTree()
{
    //ctor
}

/*bool HuffmanTree::decode(int bit,SymbolType* decodedSymbol)
{
    bit &= 1;
    currentCode <<= 1;
    currentCode |= bit;
    bitLength++;

    if(codes[currentCode].length == bitLength)
    {
        *decodedSymbol = codes[currentCode].symbol;
        currentCode = 0;
        bitLength = 0;
        return true;
    }
    //if(bitLength>high_value) throw "no code found";
    return false;
}*/

template<class SymbolType>
bool HuffmanTree<SymbolType>::decode(int bit,SymbolType* decodedSymbol)
{
    bit &= 1;
    int nbStates = states.size();
    if(nbStates==0) return false;

    currentState = states[currentState][bit];
    if(currentState >= nbStates)
    {
        *decodedSymbol = symbols[currentState-nbStates].symbol;
        currentState = 0;
        return true;
    }
    return false;
}

template<class SymbolType>
void HuffmanTree<SymbolType>::loadFromCodeLength(vector<SymbolType> alphabet, vector<int> codeLengths, int maxLength)
{
    int nbCodes = codeLengths.size();
    if(nbCodes==0) throw "Huffman error: empty code lengths";
    vector<int> lengthCount;
    /*for(int length = 0; length <= nbCodes; length++)
    {
        for(int i : codeLengths)
        {
            if(i == length)
            {
                if(lengthCount.size<= length)
                {
                    lengthCount.push_back(0);
                }
                lengthCount[length]++;
            }
        }
    }
    int maxLength = lengthCount.size()-1;*/
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

    for(unsigned int i = 0; i < codeLengths.size(); i++)
    {
        int len = codeLengths[i];
        if(len != 0)
        {
            HuffmanCode code;
            code.code = startCode[len];
            code.symbol = alphabet[i];
            code.length = len;

            code.parentState = addStates(code);
            symbols.push_back(code);
            startCode[len]++;
        }
    }

    int nbStates = states.size();
    for(unsigned int i = 0; i < symbols.size(); i++)
    {
        //todo: check still -1 or throw
        states[symbols[i].parentState][symbols[i].code&1] = i+nbStates;
    }
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
void HuffmanTree<SymbolType>::write(BitStream & in, SymbolType s)
{
    for(HuffmanCode code : symbols)
    {
        if(code.symbol == s)
        {
            //The code has to be written in reverse order !
            int offset = code.length-1;
            for(;offset>=0;offset--)
            {
                in.write((code.code>>offset)&1,1);
            }
            //in.write(code.code,code.length);
            return;
        }
    }
}
