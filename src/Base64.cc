#include "Base64.h"
#include "BitStream.h"

vector<uint8_t> b64_decode(string msg)
{

    bool skipLastByte = true;
    BitStream bs(BitStream::LeftToRight);
    for(char c : msg)
    {
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
            //cout << "end of base 64 stream reached"<<endl;//so it seems that if the last char before the '=' overlaps a new byte without willing it completely, then, those new bits have to be ignored ?
            skipLastByte = true;
            break;
        }
        else
        {
            cout << "ignoring invalid base64 character:"<<c<<endl;//todo: throw error/warning message
            continue;
        }
        //binary now contains a 6bit value
        bs.write(binary, 6);
    }
    vector<uint8_t> data = bs.getData();
    if(skipLastByte) data.pop_back();
    return data;
}
