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
    
    //mengambil block pada urutan number dari filesystem(alamat blok disimpan di dalam blok)
    bool getBlock(int number,Block *&block);
    
    //menulis block ke filesystem
    bool writeBlock(const Block *block);
    
    //mengembalikan true jika blok pada number kosong
    bool isBlockEmpty(int number);
    
    //mengembalikan alamat blok kosong, mengembalikan -1 jika tidak ada
    int getAdrEmptyBlock();
    
    //mengembalikan alamat blok kosong kedua, mengembalikan -1 jika tidak ada
    int getAdrSecondEmptyBlock();
    
    //mengembalikan jalan
    char* getPath() const;
    
    int getSize() const ;
    int getBlockCount() const ;
    int getBitmapStartBlockNum() const;
    int getIrootBlockNum() const ;
    int getDataStartBlockNum() const ;       
    
    static void debug();
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

