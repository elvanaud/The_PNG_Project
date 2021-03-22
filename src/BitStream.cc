#include "BitStream.h"
#include <cmath>

/*BitStream::BitStream():data(vector<uint8_t>())
{
    //ctor
}*/

BitStream::BitStream() : data(*new vector<BaseType>)
{
    dataManuallyAllocated = true;
    checkEndOfStream();
}

BitStream::BitStream(vector<BaseType> &d) : data(d)
{
    checkEndOfStream();
}

BitStream::~BitStream()
{
    if(dataManuallyAllocated)
    {
        delete &data;//todo: use shared pointer to ensure user can keep the returned data
    }
}

uint8_t BitStream::read8(int numBits)
{
    return read32(numBits);
}

uint16_t BitStream::read16(int numBits)
{
    return read32(numBits);
}

int BitStream::read(int numBits)
{
    return read32(numBits);
}

bool BitStream::checkEndOfStream()
{
    if(currentUnit>=data.size())
    {
        endOfStream = true;
    }
    return endOfStream;
}

void BitStream::skipToFullByte()
{
    if(bitOffset == 0) return;
    currentUnit++;
    bitOffset = 0;
    checkEndOfStream();
}

uint32_t BitStream::read32(int numBits)
{
    if(checkEndOfStream()) throw "Can't read: end of stream reached";
    BaseType d = data[currentUnit];
    uint32_t res = 0;

    const int finalMask = ((1<<numBits)-1);
    int remainingBitsInUnit = BaseBitLength-bitOffset;
    int bitsRead = remainingBitsInUnit;

    res = (d >> bitOffset);

    while(numBits > remainingBitsInUnit)
    {
        numBits -= remainingBitsInUnit;
        remainingBitsInUnit = BaseBitLength;
        currentUnit++;
        bitOffset = 0;
        if(checkEndOfStream()) break;

        res |= (uint32_t(data[currentUnit])<<bitsRead);
        bitsRead += BaseBitLength;
    }

    bitOffset += numBits;
    if(bitOffset >= BaseBitLength)
    {
        bitOffset = 0;
        currentUnit++;
        checkEndOfStream();
    }
    res &= finalMask;

    return res;
}

void BitStream::write(uint32_t d, int numBits)
{
    int remainingBitsInUnit = BaseBitLength-bitOffset;
    if(dataManuallyAllocated)//||extendData/writeExtend
    {
        //extend data vector as much as needed, otherwise: endOfStreamReached set (except if a parameter extendDataWhenWrite is setby user)
        if(data.size()==currentUnit)data.push_back(0);
        if(numBits > remainingBitsInUnit)
        {
            int nbUnits = ceil(float(numBits-remainingBitsInUnit) / BaseBitLength);
            for(;nbUnits>0;--nbUnits)
                data.push_back(0);
        }
    }
    const int dataMask = ((1<<numBits)-1);

    d &= dataMask;

    //cout<<"current unit: "<<currentUnit<<endl<<"size: "<<data.size()<<endl;;
    data[currentUnit] |= d<<bitOffset;


    while(numBits > remainingBitsInUnit)
    {
        d>>= remainingBitsInUnit;
        numBits -= remainingBitsInUnit;
        remainingBitsInUnit = BaseBitLength;
        currentUnit++;
        bitOffset = 0;

        data[currentUnit] = d;
    }
    bitOffset += numBits;

    if(bitOffset >= BaseBitLength)
    {
        bitOffset = 0;
        currentUnit++;
        //checkEndOfStream();
    }
}

vector<BaseType> BitStream::getData()
{
    return data;
}

void BitStream::reset()
{
    currentUnit = bitOffset = 0;
    endOfStream = false;
    checkEndOfStream();
}
