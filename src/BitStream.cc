#include "BitStream.h"
#include <cmath>

/*BitStream::BitStream():data(vector<uint8_t>())
{
    //ctor
}*/

BitStream::BitStream() //: data(*new vector<BaseType>)
{
    dataManuallyAllocated = true;
    //checkEndOfStream();
    reset(ReadCursor);
    reset(WriteCursor);
}

BitStream::BitStream(UnitDirection dir) : BitStream()
{
    //BitStream();
    rCursor.unitDirection = dir;
    wCursor.unitDirection = dir;

    reset(ReadCursor);
    reset(WriteCursor);
}

BitStream::BitStream(vector<BaseType> const &d) : data(d)
{
    checkEndOfStream();
}

BitStream::~BitStream()
{
    if(dataManuallyAllocated)
    {
        //delete &data;//todo: use shared pointer to ensure user can keep the returned data
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
    if(rCursor.currentUnit>=data.size())
    {
        endOfStream = true;
    }
    return endOfStream;
}

BitStream::Cursor * BitStream::parseCursorType(CursorType t)
{
    if(t == WriteCursor)
        return &wCursor;
    return &rCursor;
}

void BitStream::skipToFullByte(CursorType cType)
{
    Cursor * curs = parseCursorType(cType);

    if(curs->bitOffset == 0) return;
    curs->currentUnit++;
    curs->bitOffset = 0;
    checkEndOfStream();
}

uint32_t BitStream::read32(int numBits)
{
    if(checkEndOfStream()) throw "Can't read: end of stream reached";
    BaseType d = data[rCursor.currentUnit];
    uint32_t res = 0;

    const int finalMask = ((1<<numBits)-1);
    int remainingBitsInUnit = BaseBitLength-rCursor.bitOffset;
    int bitsRead = remainingBitsInUnit;

    res = (d >> rCursor.bitOffset);

    while(numBits > remainingBitsInUnit)
    {
        numBits -= remainingBitsInUnit;
        remainingBitsInUnit = BaseBitLength;
        rCursor.currentUnit++;
        rCursor.bitOffset = 0;
        if(checkEndOfStream()) break;

        res |= (uint32_t(data[rCursor.currentUnit])<<bitsRead);
        bitsRead += BaseBitLength;
    }

    rCursor.bitOffset += numBits;
    if(rCursor.bitOffset >= BaseBitLength)
    {
        rCursor.bitOffset = 0;
        rCursor.currentUnit++;
        checkEndOfStream();
    }
    res &= finalMask;

    return res;
}

void BitStream::writeLeftToRight(uint32_t d, int numBits)
{
    int remainingBitsInUnit = wCursor.bitOffset+1;
    if(dataManuallyAllocated)//||extendData/writeExtend
    {
        //extend data vector as much as needed, otherwise: endOfStreamReached set (except if a parameter extendDataWhenWrite is setby user)
        if(data.size()==wCursor.currentUnit)data.push_back(0);
        if(numBits > remainingBitsInUnit)
        {
            int nbUnits = ceil(float(numBits-remainingBitsInUnit) / BaseBitLength);
            for(;nbUnits>0;--nbUnits)
                data.push_back(0);
        }
        endOfStream = false;
        checkEndOfStream();
    }

    const int dataMask = ((1<<numBits)-1);
    d &= dataMask;

    while(numBits > remainingBitsInUnit)
    {
        data[wCursor.currentUnit] |= d>>(numBits-remainingBitsInUnit);

        wCursor.currentUnit++;
        wCursor.bitOffset = BaseBitLength-1;

        numBits -= remainingBitsInUnit;
        remainingBitsInUnit = BaseBitLength;
    }

    if(numBits <= remainingBitsInUnit)
    {
        data[wCursor.currentUnit] |= d<<(wCursor.bitOffset-numBits+1);
        wCursor.bitOffset-=numBits;
        if(wCursor.bitOffset<0)
        {
            wCursor.currentUnit++;
            wCursor.bitOffset = BaseBitLength-1;
        }
    }
}

void BitStream::write(uint32_t d, int numBits)
{
    if(wCursor.unitDirection == LeftToRight)
    {
        writeLeftToRight(d,numBits);
        return;
    }

    int remainingBitsInUnit = BaseBitLength-wCursor.bitOffset;
    if(dataManuallyAllocated)//||extendData/writeExtend
    {
        //extend data vector as much as needed, otherwise: endOfStreamReached set (except if a parameter extendDataWhenWrite is setby user)
        if(data.size()==wCursor.currentUnit)data.push_back(0);
        if(numBits > remainingBitsInUnit)
        {
            int nbUnits = ceil(float(numBits-remainingBitsInUnit) / BaseBitLength);
            for(;nbUnits>0;--nbUnits)
                data.push_back(0);
        }
        endOfStream = false;
        checkEndOfStream();
    }
    const int dataMask = ((1<<numBits)-1);

    d &= dataMask;
    data[wCursor.currentUnit] |= d<<wCursor.bitOffset;

    while(numBits > remainingBitsInUnit)
    {
        wCursor.currentUnit++;
        wCursor.bitOffset = 0;
        d>>= remainingBitsInUnit;

        numBits -= remainingBitsInUnit;
        remainingBitsInUnit = BaseBitLength;


        data[wCursor.currentUnit] = d;
    }
    wCursor.bitOffset += numBits;

    if(wCursor.bitOffset >= BaseBitLength)
    {
        wCursor.bitOffset = 0;
        wCursor.currentUnit++;
        //checkEndOfStream();
    }
}

vector<BaseType> BitStream::getData() const
{
    return data;
}

vector<BaseType> const & BitStream::getBufferRef() const
{
    return data;
}

void BitStream::reset(CursorType cType)
{
    Cursor * curs = parseCursorType(cType);
    curs->currentUnit = 0;
    curs->bitOffset = (curs->unitDirection == LeftToRight ? BaseBitLength-1: 0);
    endOfStream = false;
    checkEndOfStream();
}
