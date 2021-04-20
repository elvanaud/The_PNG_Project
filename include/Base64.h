#ifndef BASE64_H
#define BASE64_H

#include "Common.h"

vector<uint8_t> b64_decode(string msg);
string b64_encode(vector<uint8_t> data);
#endif // BASE64_H
