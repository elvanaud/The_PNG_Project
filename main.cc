#include "Common.h"

#include "BitStream.h"
#include "Deflate.h"
#include "HuffmanTree.h"

vector<uint8_t> b64_decode(string msg);

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
    //"Bonjour, voilà un texte que je m'apprete a compresser pour pouvoir tester mon propre programme de decompression deflate, je ne m'attend pas a ce que cela fonctionne du premier coup mais bon.... Donc je vais meubler un petit peu pour donner assez de matiere a compresser. Bonjour bonjour bonjour voila voila lolilol C'est vraiment ridicule mais bon.........."

    //data = b64_decode(compressedMsg);
    string outMsg;
    for(uint8_t c : b64_decode("Qm9uam91ciwgdm9pY2kgdW4gbWVzc2FnZSBxdWkgc2VyYSwgbm9ybWFsbGVtZW50LCBqZSBsJ2VzcGVyZSBkZWNvZGUgcGFyIG1vbiBkZWNvZGV1ciBiYXNlIDY0ICEhIQ=="))
        outMsg += char(c);
    cout << outMsg << endl;
    assert(outMsg == "Bonjour, voici un message qui sera, normallement, je l'espere decode par mon decodeur base 64 !!!");
}

vector<uint8_t> b64_decode(string msg)
{
    vector<uint8_t> data;
    BitStream bs(BitStream::LeftToRight);
    for(char c : msg)
    //for(int i = 3; i < msg.size(); i+=4)
    {

        //for(int j = 0; j < 4; j++)
        {
            //char c = msg[i-j];
            uint8_t binary = 0;
            if(c >= 'A' && c <= 'Z')
                binary = c - 'A';
            else if(c >= 'a' && c <= 'z')
                binary = c-'a'+26;
            else if(c >= '0' && c <= '9')
                binary = c-'0'+52;
            else if(c == '+')
                binary = 62;
            else if(c == '/')
                binary = 63;
            else if(c == '=')
            {
                cout << "end of base 64 stream reached"<<endl;
                break;
            }
            else
            {
                cout << "ignoring invalid base64 character:"<<c<<endl;
                continue;
            }
            //binary now contains a 6bit value
            bs.write(binary, 6);
        }
        //for(uint8_t d : bs.getData())
            //data.push_back(d);


    }
    return bs.getData();
}

int main()
{
    cout << "The PNG Project" << endl;

    vector<uint8_t> data;
    string compressedMsg = "eJxVUEFuAyEMvOcVvuUS5RFpP+KAUxFhTMGsqr6mf+nHOu62h7XARtbMeMzN2tPWuNBmpX5/0Wrk8uFC70voKaRn7n0IGkzJFM85ZVAHJxJYA4Tp6Kk16sMAifI2WFUox/knFiCyPCq7XEK8/eq7S8vUecaIfXCSyvSwlhwUoPKCpGjBlGSrk3KZdLd2RZxegQu1LZoq614Bwx5dvDjy2t3mUBrE8P8ZtpQdese9rnTb/yPEDzW+h/9ytVpwTy9nLE7b4KLSnEbJJa0qB3d7/ABp/IFv";
    //compressedMsg = "eJxzys/Lyi8tAgAK5gLg";
    compressedMsg = "VVBBbgMhDPyKb7lEeUTSjzjgVEQYEzCrqq/pX/qxjrvtYS2wkTUzHnO19rQ1zrRZqd9ftBq5fLjQawk9hfTEvQ9BgymZ4jmnDOrgRAJrgDAdPbVGfRggUd4HqwrlOP/EAkSWR2WXc4i3X313aZk6zxixD05SmR7WkoMCVF6QFC2Ykmx1Ui6T7tYuCHoDLtS2aKqsewUMe3Tx4shrd5tDaRDD/2fYUnboHfe60HX/jxA/1Pge/svVasGl2wmL0za4qDSnUXJJq8rB3R4/";
    data = b64_decode(compressedMsg);

    try
    {
        test();

        //vector<uint8_t> data = {0b0000'0001,1,0,0xFE,0xFF,42};//uncompressed block of one byte
                            //0b00000'101,0b000'00000,0b0'000'000'0,//dynamic huffman
                            //0b000'001'00};
        BitStream bs(data);
        Deflate deflator;
        //BitStream os;


        BitStream& os(deflator.uncompress(bs));
        //assert(os.getData().size()==1);
        //assert(os.getData()[0] == 42);
        vector<uint8_t> result = os.getData();
        for(uint8_t r : result)
            cout << char(r);
    }
    catch(const char * msg)
    {
        cerr << "Exception: "<<msg<<endl;
    }


    return 0;
}
