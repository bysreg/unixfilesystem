#include "Filesystem.h"
#include "Block.h"

#ifndef INODE_H
#define	INODE_H

class Inode {
public:
    static const int MAX_ADDRESS_COUNT = 1022;
    static const int FILE = 0;
    static const int DIR = 1;
    
    Inode(int blockaddress, Filesystem filesystem);
    Inode(const Inode& orig);
    virtual ~Inode();
    
    //mengambil tipe inode tersebut
    int getType() const ;
    //mengambil data address blok pada slot, mengembalikan -1 jika tidak ada address disitu
    int getDataAddress(int slot) const;
    //mengambil address blok lain, mengembalikan -1 jika tidak ada address disitu 
    int getOtherAddressBlock() const;
    
    //membangun inode dan mengembalikan blok representasi inode tersebut(BELUM DISAVE ke disk!)
    static Block consInode(Filesystem fs, int type, vector<int> dataaddress, int otheraddressblock);
    
    void setType(int type);
    void setDataAddress(int slot, int blockAddress);
    void setOtherAddressBlock(int otherAddressBlock);

private:
    //file attributes    
    int type;

    //alamat block yang memuat data file ini
    int dataaddress[MAX_ADDRESS_COUNT];

    //alamat block yang memuat address block
    int otheraddressblock;

};

#endif	/* INODE_H */

