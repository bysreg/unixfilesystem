#include "ByteUtil.h"
#include <iostream>
using namespace std;
vector<byte> ByteUtil::intToBytes(int val) {
     vector<unsigned char> arrbytes(4);
     for (int i = 0; i < 4; i++)
         arrbytes[3 - i] = (val >> (i * 8));
     return arrbytes;
}

int ByteUtil::bytesToInt(const vector<byte> val) {
    int result = 0;
    for(int i=0;i<val.size();i++) {
        result = (result << 8) + val[i];
    }
    return result;
}

int ByteUtil::bytesToInt(const byte* val) {
    int result = 0;
    for(int i=0;i<4;i++) {
        result = (result << 8) + val[i];
    }    
    return result;    
}