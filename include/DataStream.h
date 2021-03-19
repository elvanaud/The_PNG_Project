#ifndef DATASTREAM_H
#define DATASTREAM_H

#include <vector>
using std::vector;
#include <cstdint>

class DataStream
{
public:
    //DataStream();
    DataStream(vector<uint8_t> &d);
    uint8_t     read8(int numBits);
    uint16_t    read16(int numBits);
    uint32_t    read32(int numBits);
    //int         read(int numBits);

    bool endOfStream = false;
    bool checkEndOfStream();

    void reset();
private:
    vector<uint8_t> &data;//careful
    int currentByte = 0;
    int offsetWithinByte = 0;
};

#endif // DATASTREAM_H
