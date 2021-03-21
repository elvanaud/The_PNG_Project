#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "Common.h"

//template<class BaseType,unsigned int BaseBitLength>//TODO
using BaseType = uint8_t;
const unsigned int BaseBitLength = 8;
//using MaxBufferType = uint32_t; //TODO: use that
class BitStream
{
public:
    BitStream();
    ~BitStream();
    BitStream(vector<BaseType> &d);
    uint8_t     read8(int numBits);
    uint16_t    read16(int numBits);
    uint32_t    read32(int numBits);
    int         read(int numBits);

    void write(uint32_t d, int numBits);

    bool endOfStream = false;
    bool checkEndOfStream();

    void reset();
    void skipToFullByte();

    vector<BaseType> getData(); //outputs a copy of written data at the moment of calling
private:
    vector<BaseType> &data;//careful
    unsigned int currentUnit = 0;
    unsigned int bitOffset = 0;

    bool dataManuallyAllocated = false;
};

#endif // BITSTREAM_H
