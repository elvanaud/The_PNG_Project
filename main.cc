#include "Common.h"

#include "BitStream.h"
#include "Deflate.h"

void test()
{
    vector<uint8_t> data = {0b0111'0110,0b1101'0010,0b1010'1110,0b1100'1010};
    BitStream ds(data);

    cout << "Launching BitStream test..." << endl;
    int32_t r = ds.read32(5);
    assert(r==0b1'0110);
    assert(ds.read32(4) == 0b0'011);
    assert(ds.read32(17)==0b10'1010'1110'1101'001);
    assert(ds.read32(5)==0b100'10);
    assert(!ds.endOfStream);
    assert(ds.read32(1)==0b1);
    assert(ds.endOfStream);
    cout << "BitStream test pass !" << endl;

    BitStream os;
    os.write(0b1111'0101,4);
    os.write(0b00,2);
    os.write(0b1001'11,6);
    os.write(0b1110'1111'1111'0110,16);
    os.write(0b110'1,1);
    vector<uint8_t> outData = os.getData();//returns a copy of current outputed data
    assert(outData.size()==4);
    assert(outData[0]==0b11'00'0101);
    assert(outData[1]==0b0110'1001);
    assert(outData[2]==0b1111'1111);
    assert(outData[3]==0b0001'1110);
}

int main()
{
    cout << "The PNG Project" << endl;

    test();

    vector<uint8_t> data = {0b0000'0001,1,0,0xFE,0xFF,42};//uncompressed block of one byte
    BitStream bs(data);
    Deflate deflator;
    //BitStream os;
    try
    {
        BitStream& os(deflator.uncompress(bs));
        assert(os.getData().size()==1);
        assert(os.getData()[0] == 42);
    }
    catch(const char * msg)
    {
        cerr << "Exception: "<<msg<<endl;
    }


    return 0;
}
