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
            string err = "ignoring invalid base64 character: ";
            err += c;
            throw err; //maybe I should keep executing and only throw at the end, so that random invalid chars don't interupt the whole process for nothing
            continue;
        }
        //binary now contains a 6bit value
        bs.write(binary, 6);
    }
    vector<uint8_t> data = bs.getData();
    if(skipLastByte) data.pop_back();
    return data;
}

string b64_encode(vector<uint8_t> data)
{
    BitStream bs(data, BitStream::LeftToRight);
    string res;

    auto mapBinaryToChar = [&res](uint8_t binary)
    {
        if(binary < 26) {
            res += ('A'+binary);
        } else if(binary < 52) {
            res += ('a'+binary-26);
        } else if(binary < 62) {
            res += ('0'+binary-52);
        } else if(binary == 62) {
            res += '+';
        } else if(binary == 63) {
            res += '/';
        }
    };
    try
    {
        for(;;) //process until end of stream reached
        {
            uint8_t binary = bs.read(6);
            mapBinaryToChar(binary);
        }
    }
    catch(const char* msg) //eos reached
    {
        int remaining = res.size()%4;
        if(remaining > 0) //needs padding, and the last char hasn't been treated yet
        {
            uint8_t b = (data[data.size()-1]&0x03)<<4; //reconstitute the last 6bit binary value
            mapBinaryToChar(b);
            for (int i = 0;i < 4-remaining-1;i++) res += '=';
        }
        return res;
    }

    return res;
}
