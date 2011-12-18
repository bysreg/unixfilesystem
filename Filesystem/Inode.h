#include "Filesystem.h"
#include "Block.h"

#ifndef INODE_H
#define	INODE_H

class Inode {
public:
    static const int MAX_ADDRESS_COUNT = 1022;
    static const int FILE = 0;
    static const int DIR = 1;    
    
    //membuat objek inode dari alamat blok pada filesystem
    Inode(int blockaddress, Filesystem filesystem);
    Inode(const Inode& orig);
    virtual ~Inode();
    
    //mengambil tipe inode tersebut
    int getType() const ;
    //mengambil data address blok pada slot, mengembalikan -1 jika tidak ada address disitu atau melebihi slot maksimum
    int getDataAddress(int slot) const;
    //mengambil file size(direktori selalu mengembalikan 0)
    int getFileSize() const;
    //mengambil alamat inode ini
    int getAddress() const;
    
    //membangun inode dan mengembalikan alamat blok representasi inode tersebut, mengembalikan -1 jika tidak ada size
    static int consInode(Filesystem *fs, int type, vector<int> dataaddress, int argfilesize);
        
//    void setDataAddress(int slot, int blockAddress);

private:
    //file attributes    
    int type;

    //alamat block yang memuat data file ini
    int dataaddress[MAX_ADDRESS_COUNT];

    //file size suatu file
    int filesize;

    //alamat block inode ini
    int address;
};

#endif	/* INODE_H */

