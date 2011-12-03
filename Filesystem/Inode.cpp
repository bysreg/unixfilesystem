#include "Inode.h"

Inode::Inode(int blockaddress, Filesystem filesystem) {
    Block *block;
    byte* pbyte;
    filesystem.getBlock(blockaddress, block);
    //ambil empat byte pertama untuk atribut file
    pbyte = (*block).getBytes(0, 4);
    type = ByteUtil::bytesToInt(pbyte);
    delete[] pbyte;
    //ambil empat byte kedua untuk alamat address blok lain
    pbyte = block->getBytes(4,4);
    otheraddressblock = ByteUtil::bytesToInt(pbyte);
    delete[] pbyte;
    //ambil tiap empat byte berikutnya untuk alamat blok file disimpan
    for (int i = 8; i < MAX_ADDRESS_COUNT; i++) {
        pbyte = block->getBytes(i, 4);
        dataaddress[i - 8] = ByteUtil::bytesToInt(pbyte);
        delete[] pbyte;
    }
}

Inode::Inode(const Inode& orig) {
    this->type = orig.getType();
    for(int i=0;i<MAX_ADDRESS_COUNT;i++) {
        this->dataaddress[i] = orig.getDataAddress(i);
    }
    this->otheraddressblock = orig.getOtherAddressBlock();    
}

Inode::~Inode() {
}

int Inode::getType() const {
    return type;
}

int Inode::getDataAddress(int slot) const {
    if(dataaddress[slot]==0) {//0 itu artinya tidak ada address(alamat 0 sudah pasti superblock)
        return -1;
    }
    return dataaddress[slot];
}

int Inode::getOtherAddressBlock() const {
    if(otheraddressblock==0) {
        return -1;
    }
    return otheraddressblock;
}

Block Inode::consInode(Filesystem fs, int type, vector<int> dataaddress, int otheraddressblock) {
    int bytecount=0;
    byte data[Block::BLOCK_SIZE];
    vector<byte> b_type = ByteUtil::intToBytes(type);
    //tulis empat byte pertama untuk atribut file
    for(int i=0;i<b_type.size();i++) {
        data[bytecount] = b_type[i];
        bytecount++;
    }
    //tulis empat byte kedua untuk alamat address blok lain
    vector<byte> b_otheraddressblock = ByteUtil::intToBytes(otheraddressblock);
    for(int i=0;i<b_otheraddressblock.size();i++) {
        data[bytecount] = b_otheraddressblock[i];
        bytecount++;
    }
    //tulis empat byte ketiga dan seterusnya untuk alamat blok file disimpan(sebanyak dataaddress)
    for(int i=0;i<dataaddress.size();i++) {        
        vector<byte> b_dataaddress = ByteUtil::intToBytes(dataaddress[i]);        
        for(int j=0;j<b_dataaddress.size();j++) {
            data[bytecount] = b_dataaddress[j];
            bytecount++;
        }
    }
    for(int i=bytecount;i<Block::BLOCK_SIZE;i++) {
        data[bytecount] = 0;
        bytecount++;
    }
    
    Block block(fs.getAdrEmptyBlock(),data);
    return block;
}

/*
int main() {
    Inode::consInode();
}
*/