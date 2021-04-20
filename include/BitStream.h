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
    enum UnitDirection{LeftToRight, RightToLeft};
    BitStream(UnitDirection dir);
    BitStream();
    ~BitStream();
    BitStream(vector<BaseType> const &d, UnitDirection dir=RightToLeft);
    uint8_t     read8(int numBits);
    uint16_t    read16(int numBits);
    uint32_t    read32(int numBits);
    int         read(int numBits);

    void write(uint32_t d, int numBits);

    bool endOfStream = false;//todo: replace this with a closedStream bool and a setter
    bool checkEndOfStream();//only works with the read cursor

    enum CursorType {ReadCursor,WriteCursor};
    void reset(CursorType cType);
    void skipToFullByte(CursorType cType);//add a method to know when writing that the current byte is full

    vector<BaseType> getData() const; //outputs a copy of written data at the moment of calling
    vector<BaseType> const & getBufferRef() const;
private:
    vector<BaseType> data;
    struct Cursor
    {
        unsigned int currentUnit = 0;
        int bitOffset = 0;
        UnitDirection unitDirection = RightToLeft;
    };
    Cursor rCursor;
    Cursor wCursor;

    Cursor * parseCursorType(CursorType t);

    bool dataManuallyAllocated = false;//todo: rename writeExtend
    void writeLeftToRight(uint32_t d, int numBits);
};

#endif // BITSTREAM_H
