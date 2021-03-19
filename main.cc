#include <iostream>
#include <cassert>
using std::cout;
using std::endl;

#include "DataStream.h"

void test()
{
    vector<uint8_t> data = {0b0111'0110,0b1101'0010,0b1010'1110,0b1100'1010};
    DataStream ds(data);

    cout << "DataStream test" << endl;
    int32_t r = ds.read32(5);
    assert(r==0b1'0110);
    assert(ds.read32(4) == 0b0'011);
    assert(ds.read32(17)==0b10'1010'1110'1101'001);
    assert(ds.read32(5)==0b100'10);
    assert(!ds.endOfStream);
    assert(ds.read32(1)==0b1);
    assert(ds.endOfStream);
}

int main()
{
    cout << "The PNG Project" << endl;

    //test();

    return 0;
}
