#include <string>
#include "ByteUtil.h"
#include "Block.h"
#include <cstring>
#include <cmath>

using namespace std;

#ifndef FILESYSTEM_H
#define	FILESYSTEM_H

class Filesystem {
public:    
    //size maksimum filesystem(dalam KB)
    static const int MAX_SIZE=65535;       
        
    Filesystem(string path);
    Filesystem(const Filesystem& orig);    
    virtual ~Filesystem();    
    
    //memformat device pada path(size dalam KB)
    static bool format(string path, unsigned int size);
    
    //mengambil block pada urutan number dari filesystem
    bool getBlock(int number,Block *&block);
    
    //mengembalikan true jika blok pada number kosong
    //bool isBlockEmpty(int number);
    
    int getSize();
    int getBlockCount();
    int getBitmapStartBlockNum();
    int getIrootBlockNum();
    int getDataStartBlockNum();
    
private:
    //path filesystem
    char *path;
    //size filesystem
    int size;
    //jumlah block yang ada di filesystem
    int nblock;
    //alamat(block number) mulai block bitmap
    int bitmapStartBlockNum;
    //inode root block number
    int irootBlockNum;
    //alamat(block number) mulai block data
    int dataStartBlockNum;
};

#endif	/* FILESYSTEM_H */

