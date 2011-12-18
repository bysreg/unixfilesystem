#include "Inode.h"
#include <iostream>
#include <cstdio>

using namespace std;

Inode::Inode(int blockaddress, Filesystem filesystem) {
    Block *block;
    byte* pbyte;
    filesystem.getBlock(blockaddress, block);
    
    this->address = blockaddress;
    
    //ambil empat byte pertama untuk atribut file
    pbyte = (*block).getBytes(0, 4);
    type = ByteUtil::bytesToInt(pbyte);
    delete[] pbyte;
    //ambil empat byte kedua untuk filesize
    pbyte = block->getBytes(4, 4);
    filesize = ByteUtil::bytesToInt(pbyte);
    delete[] pbyte;
    //ambil tiap empat byte berikutnya untuk alamat blok file disimpan
    for(int i=0;i<MAX_ADDRESS_COUNT;i++) {
        pbyte = block->getBytes(i*4+8,4);        
        dataaddress[i] = ByteUtil::bytesToInt(pbyte);        
        delete[] pbyte;
    }
}

Inode::Inode(const Inode& orig) {
    this->address = orig.getAddress();
    this->type = orig.getType();
    for (int i = 0; i < MAX_ADDRESS_COUNT; i++) {
        this->dataaddress[i] = orig.getDataAddress(i);
    }
    this->filesize = orig.getFileSize();
}

Inode::~Inode() {
}

int Inode::getType() const {
    return type;
}

int Inode::getDataAddress(int slot) const {
    if(slot>=MAX_ADDRESS_COUNT) {
        return -1;
    }
    if (dataaddress[slot] == 0) {//0 itu artinya tidak ada address(alamat 0 sudah pasti superblock)
        return -1;
    }    
    return dataaddress[slot];
}

int Inode::getFileSize() const {
    if (filesize == 0) {
        return -1;
    }
    return filesize;
}

int Inode::getAddress() const {
    return this->address;
}

int Inode::consInode(Filesystem *fs, int type, vector<int> dataaddress, int argfilesize) {
    int bytecount = 0;
    byte data[Block::BLOCK_SIZE];
    vector<byte> b_type = ByteUtil::intToBytes(type);
    //tulis empat byte pertama untuk atribut file
    for (int i = 0; i < b_type.size(); i++) {
        data[bytecount] = b_type[i];
        bytecount++;
    }
    //tulis empat byte kedua untuk size file yang ditunjuk oleh inode ini
    vector<byte> b_argfilesize = ByteUtil::intToBytes(argfilesize);
    for (int i = 0; i < b_argfilesize.size(); i++) {
        data[bytecount] = b_argfilesize[i];
        bytecount++;
    }    
    //tulis empat byte ketiga dan seterusnya untuk alamat blok file disimpan(sebanyak dataaddress)
    for (int i = 0; i < dataaddress.size(); i++) {
        vector<byte> b_dataaddress = ByteUtil::intToBytes(dataaddress[i]);
        for (int j = 0; j < b_dataaddress.size(); j++) {
            data[bytecount] = b_dataaddress[j];
            bytecount++;
        }
    }        

    for (int i = bytecount; i < Block::BLOCK_SIZE; i++) {
        data[bytecount] = 0;
        bytecount++;
    }

    int emptyblocknum;
    if ((emptyblocknum = fs->getAdrEmptyBlock()) != -1) {//dapat alamat blok yang tidak kosong
        Block *block = new Block(emptyblocknum, data);        
        fs->writeBlock(block);
        return block->number;
    } else {
        return -1;
    }
}


//void Inode::setDataAddress(int slot, int blockAddress) {
//    dataaddress[slot] = blockAddress;
//}
//

//int main() {
//    printf("tes inode\n");
//    cout<<"mount : "<<(Filesystem::format("device.txt",(unsigned int)1024))<<endl;  
//    Filesystem fs("device.txt"); 
//    vector<int> dataaddress;
//    dataaddress.push_back(999);
//    printf("tes buat inode : %d\n",Inode::consInode(&fs, Inode::DIR, dataaddress, 10));
//    //Filesystem::debug();        
//    Inode inode(3, fs);
//    printf("tipe : %d\n", inode.getType());
//    printf("alamat blok file pertama : %d\n", inode.getDataAddress(0));
//    printf("alamat blok yang memuat address blok : %d\n", inode.getOtherAddressBlock());    
//}
