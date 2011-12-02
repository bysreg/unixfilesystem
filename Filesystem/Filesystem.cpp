#include "Filesystem.h"
#include <fstream>
#include <iostream>

Filesystem::Filesystem(string path) {    
    this->path = new char[path.size()+1];
    strcpy(this->path,path.c_str());    
    Block *superblock;
    getBlock(0,superblock);    
    byte *sizeInBytes = (*superblock).readBlock(0,4);//ambil size    
    size = ByteUtil::bytesToInt(sizeInBytes);    
    nblock = size*1000/(Block::BLOCK_SIZE);   
    bitmapStartBlockNum = 1;
    irootBlockNum = (int)ceil(nblock/Block::BLOCK_SIZE) + 2;
    dataStartBlockNum = irootBlockNum + 1;
    delete[](sizeInBytes);
    delete(superblock);
}

Filesystem::Filesystem(const Filesystem& orig) {
}

Filesystem::~Filesystem() {
}

bool Filesystem::format(string path, unsigned int size) {    
    int nblock = size*1000/(Block::BLOCK_SIZE);        
    int BitmapEndBlockNumber = (int)ceil(nblock/Block::BLOCK_SIZE) + 1;//nomor ujung blok bitmap     
    ofstream fout(path.c_str(),ios::out|ios::trunc|ios::binary);
    
    if(!fout.is_open()) {
        return false;
    }
    
    //tulis size ke dalam 4 byte pertama blok 0(superblock)
    vector<byte> sizeInByte = ByteUtil::intToBytes(size);
    for(int i=0;i<sizeInByte.size();i++) {
        fout<<sizeInByte[i];
    }
    for(int i=0;i<Block::BLOCK_SIZE-4;i++) {
        fout<<(byte)0;
    }    
    
    //tulis bitmap di blok ke 1 sampai ceil(nblock/Block::BLOCK_SIZE);
    for(int i=0;i<nblock;i++) {
        if(i<=BitmapEndBlockNumber+1) {            
            fout<<(byte)1;//blok superblock,bitmap,inode root sudah diisi
        }else{
            fout<<(byte)0;
        }
    }
    //jika blok bitmap masih sisa, penuhin dengan angka 0
    if(nblock % Block::BLOCK_SIZE > 0) {
        for(int i=0;i<Block::BLOCK_SIZE - (nblock % Block::BLOCK_SIZE);i++) {
            fout<<(byte)0;
        }
    }
    
    //blok ke ceil(nblock/Block::BLOCK_SIZE) + 1 untuk inode root
    for(int i=0;i<Block::BLOCK_SIZE;i++) {
        fout<<(byte)0;
    }
    
    //isi blok data dengan 0 semua
    for(int iblock=BitmapEndBlockNumber+2;iblock<nblock;iblock++) {
        for(int i=0;i<Block::BLOCK_SIZE;i++) {
            fout<<(byte)0;
        }
    }
        
    fout.close();
    return true;
}

bool Filesystem::getBlock(int number, Block *&block) {        
    ifstream fin(path,ios::in | ios::binary);
    if(!fin.is_open() || number<0) {
        return false;
    }
    byte data[Block::BLOCK_SIZE];        
    fin.seekg(Block::BLOCK_SIZE*number,ios::beg);                       
    for(int i=0;i<Block::BLOCK_SIZE;i++) {
        fin>>data[i];                
    }
    block = new Block(number,data);        
    fin.close();
    return true;
}

int Filesystem::getSize(){
    return size;
}

int Filesystem::getBlockCount() {
    return nblock;
}

int Filesystem::getBitmapStartBlockNum() {
    return bitmapStartBlockNum;
}

int Filesystem::getIrootBlockNum() {
    return irootBlockNum;
}

int Filesystem::getDataStartBlockNum() {
    return dataStartBlockNum;
}
