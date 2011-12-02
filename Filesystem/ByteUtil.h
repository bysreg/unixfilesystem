#include <vector>
#include "byte.h"

using namespace std;

#ifndef BYTEUTIL_H
#define	BYTEUTIL_H

class ByteUtil {
public:
    
    //mengembalikan representasi val dalam array of bytes(big endian)
    static vector<byte> intToBytes(int val);
    
    //mengembalikan representasi val(big endian 4 byte) dalam integer
    static int bytesToInt(const vector<byte> val);
    
    //mengembalikan representasi val(big endian 4 byte) dalam integer
    static int bytesToInt(const byte* val);
    
private:

};

#endif	/* BYTEUTIL_H */

