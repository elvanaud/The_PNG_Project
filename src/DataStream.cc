#include "DataStream.h"

/*DataStream::DataStream():data(vector<uint8_t>())
{
    //ctor
}*/

DataStream::DataStream(vector<uint8_t> &d) : data(d)
{
    checkEndOfStream();
}

uint8_t DataStream::read8(int numBits)
{
    return read32(numBits);
}

uint16_t DataStream::read16(int numBits)
{
    return read32(numBits);
}

bool DataStream::checkEndOfStream()
{
    if(currentByte>=data.size())
    {
        endOfStream = true;
    }
    return endOfStream;
}

uint32_t DataStream::read32(int numBits)
{
    if(checkEndOfStream()) return 0;
    uint8_t d = data[currentByte];
    uint32_t res = 0;

    const int finalMask = ((1<<numBits)-1);
    int remainingBitsInByte = 8-offsetWithinByte;
    int bitsRead = remainingBitsInByte;

    res = (d >> offsetWithinByte);

    while(numBits > remainingBitsInByte)
    {
        numBits -= remainingBitsInByte;
        remainingBitsInByte = 8;
        currentByte++;
        offsetWithinByte = 0;
        if(checkEndOfStream()) break;

        res |= (uint32_t(data[currentByte])<<bitsRead);
        bitsRead += 8;
    }

    offsetWithinByte += numBits;
    if(offsetWithinByte >= 8)
    {
        offsetWithinByte = 0;
        currentByte++;
        checkEndOfStream();
    }
    res &= finalMask;

    return res;
}

void DataStream::reset()
{
    currentByte = offsetWithinByte = 0;
    checkEndOfStream();
}
