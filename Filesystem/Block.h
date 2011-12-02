#include "byte.h"

#ifndef BLOCK_H
#define	BLOCK_H

class Block {
public:
    //size satu blok(dalam byte)
    static const int BLOCK_SIZE = 4096;    
    
    Block(int number,byte *data);
    Block(const Block& orig);
    virtual ~Block();
    
    //mengembalikan seluruh byte data yang tersimpan dalam blok ini
    byte* readBlock();
    
    //mengambil hanya byte pada blok ini yang terletak pada posisi offset sebanyak
    //count byte
    byte* readBlock(int offset, int count);
    
    //menulis sejumlah count byte src ke dalam blok(asumsi count tidak melebihi BLOCK_SIZE)
    //TODO : belum dites
    void writeBlock(const char *src, int count);    
    
    //menulis array byte src mulai dari index idx(index byte blok paling kecil adalah 0)
    //TODO : belum dites
    void writeBlock(const char *src, int count, int idx);
    
    //mengambil satu byte pada posisi pos pada blok ini
    byte getByte(int pos);  
    
private:
    byte data[BLOCK_SIZE];    
    const int number;
};

#endif

