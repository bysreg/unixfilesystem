#include "byte.h"

#ifndef BLOCK_H
#define	BLOCK_H

class Block {
public:
    //size satu blok(dalam byte)
    static const int BLOCK_SIZE = 4096;   
    
    //sengaja memang bisa langsung diubah di luar kelas(publik)
    byte data[BLOCK_SIZE];    
    const int number;//alamat blok ini
    
    Block(int number,const byte *data);
    Block(const Block& orig);
    virtual ~Block();
    
    //mengambil hanya byte pada blok ini yang terletak pada posisi offset sebanyak
    //count byte
    byte* getBytes(int offset, int count);

    //mengambil satu byte pada posisi pos pada blok ini
    byte getByte(int pos) const ;  
    
private:    
};

#endif

