#include "Tests.h"
#include "BitStream.h"
#include "HuffmanTree.h"
#include "Base64.h"

#include <bitset>

void test()
{
    vector<uint8_t> data = {0b0111'0110,0b1101'0010,0b1010'1110,0b1100'1010};
    BitStream ds(data);

    cout << "Launching BitStream read test..." << endl;
    int32_t r = ds.read32(5);
    assert(r==0b1'0110);
    assert(ds.read32(4) == 0b0'011);
    assert(ds.read32(17)==0b10'1010'1110'1101'001);
    assert(ds.read32(5)==0b100'10);
    assert(!ds.endOfStream);
    assert(ds.read32(1)==0b1);
    assert(ds.endOfStream);
    cout << "BitStream read test pass !" << endl;

    cout << "Launching BitStream write test..." << endl;
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
    cout << "BitStream write test pass !" << endl;

    cout << "HuffmanTree test..."<<endl;
    HuffmanTree<char> tree;
    vector<char> alphabet = {'A','B','C','D','E'};
    vector<int> codeLengths = {2,1,3,3};//ignore E
    tree.loadFromCodeLength(alphabet,false,codeLengths,3);
    BitStream htStream;
    string msg = "ADCBACCDBC";
    for(char letter : msg)
        tree.write(htStream,letter);
    //htStream.reset();
    vector<uint8_t> htRes= htStream.getData();
    assert(htRes.size()==3);
    assert(htRes[0]==0b011'111'01);
    assert(htRes[1]==0b11'011'01'0);
    assert(htRes[2]==0b011'0'111'0);
    //htStream.reset();
    for(char letter: msg)
    {
        //cout<<letter;
        assert(tree.readNext(htStream)==letter);
    }
    cout << "HuffmanTree test pass !"<<endl;

    cout << "Launching BitStream Left to Right write test..." << endl;
    BitStream rbs(BitStream::LeftToRight);
    rbs.write(0b1010'1111,8);
    rbs.write(0b1110,4);
    rbs.write(0b0110'11,6);
    rbs.write(0b0001'1001,8);
    rbs.write(0b110,3);
    rbs.write(0b010,3);
    data = rbs.getData();
    assert(data.size()==4);
    assert(data[0] == 0b1010'1111);
    assert(data[1] == 0b1110'0110);
    assert(data[2] == 0b11'0001'10);
    assert(data[3] == 0b01'110'010);
    cout << "BitStream Left to Right write test pass !" << endl;

    cout << "Launching BitStream Left to Right read test..." << endl;
    data = {0b0111'0110,0b1101'0010,0b1010'1110,0b1100'1010};
    BitStream ribs(data, BitStream::LeftToRight);
    assert(ribs.read(5) == 0b0111'0);
    assert(ribs.read(5) == 0b110'11);
    assert(ribs.read(10) == 0b01'0010'1010);
    assert(ribs.read(2) == 0b11);
    assert(ribs.read(3) == 0b10'1);
    assert(ribs.read(7) == 0b100'1010);
    cout << "BitStream Left to Right read test pass !"<< endl;

    cout << "Launching Base64 decode test..." << endl;
    string realMsg = "Bonjour, voici un message qui sera, normallement, je l'espere decode par mon decodeur base 64 !!!";
    string encodedMsg = "Qm9uam91ciwgdm9pY2kgdW4gbWVzc2FnZSBxdWkgc2VyYSwgbm9ybWFsbGVtZW50LCBqZSBsJ2VzcGVyZSBkZWNvZGUgcGFyIG1vbiBkZWNvZGV1ciBiYXNlIDY0ICEhIQ==";
    string outMsg;
    for(uint8_t c : b64_decode(encodedMsg))
    {
        outMsg += char(c);
    }
    assert(outMsg == realMsg);
    cout << "Base64 decode test pass !" << endl;

    cout << "Launching Base64 encode test..." << endl;
    vector<uint8_t> dataToEncode;
    for(char c : realMsg)
    {
        dataToEncode.push_back((uint8_t)c);
    }
    string encoded = b64_encode(dataToEncode);
    assert(encoded == encodedMsg);
    cout << "Base64 encode test pass !" << endl;
}

bool mainTests()
{
    bool allTestsPassed = true;
    //testBase64();
    test();
    return allTestsPassed;
}
