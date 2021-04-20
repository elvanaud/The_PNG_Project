#include "Common.h"

#include "BitStream.h"
#include "Deflate.h"
#include "HuffmanTree.h"
#include "Base64.h"
#include "tests/Tests.h"

int main()
{
    cout << "The PNG Project" << endl;
    mainTests();

    vector<uint8_t> data;
    string compressedMsg = "eJxVUEFuAyEMvOcVvuUS5RFpP+KAUxFhTMGsqr6mf+nHOu62h7XARtbMeMzN2tPWuNBmpX5/0Wrk8uFC70voKaRn7n0IGkzJFM85ZVAHJxJYA4Tp6Kk16sMAifI2WFUox/knFiCyPCq7XEK8/eq7S8vUecaIfXCSyvSwlhwUoPKCpGjBlGSrk3KZdLd2RZxegQu1LZoq614Bwx5dvDjy2t3mUBrE8P8ZtpQdese9rnTb/yPEDzW+h/9ytVpwTy9nLE7b4KLSnEbJJa0qB3d7/ABp/IFv";
    //compressedMsg = "eJxzys/Lyi8tAgAK5gLg";
    compressedMsg = "VVBBbgMhDPyKb7lEeUTSjzjgVEQYEzCrqq/pX/qxjrvtYS2wkTUzHnO19rQ1zrRZqd9ftBq5fLjQawk9hfTEvQ9BgymZ4jmnDOrgRAJrgDAdPbVGfRggUd4HqwrlOP/EAkSWR2WXc4i3X313aZk6zxixD05SmR7WkoMCVF6QFC2Ykmx1Ui6T7tYuCHoDLtS2aKqsewUMe3Tx4shrd5tDaRDD/2fYUnboHfe60HX/jxA/1Pge/svVasGl2wmL0za4qDSnUXJJq8rB3R4/";
    data = b64_decode(compressedMsg);

    try
    {
        BitStream bs(data);
        Deflate deflator;
        //BitStream os;

        BitStream& os(deflator.uncompress(bs));

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
