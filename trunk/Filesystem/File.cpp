#include <stdlib.h>
#include "File.h"
#include <iostream>
#include <cstdio>

using namespace std;

File::File(int iAddr, Filesystem fs) {    
    Inode inode(iAddr, fs);
    int bytecount = 0, slot=0, dataAddr;
    Block* block;    
    this->type = inode.getType();
    if (type == Inode::DIR) {
        while ((dataAddr = inode.getDataAddress(slot)) != -1) {
            fs.getBlock(dataAddr, block);
            for (int i = 0; i < Block::BLOCK_SIZE; i++) {                
                //printf("byte %d : %d\n",i,block->getByte(i));
                data.push_back(block->getByte(i));
            }
            delete block;
            slot++;
        }
        this->size = data.size();
        char c_name[64];
        char c;
        int i=0;        
        
        while((c=data[i])!='\0') {
            c_name[i]=c;            
            i++;
        }
        c_name[i]='\0';        
        this->name = c_name;
    } else if (type == Inode::FILE) {
        //
    }
    return ;
}

File::File(const File& orig) {

}

File::~File() {
}

int File::getSize() {
    return this->size;
}

byte File::getDataByte(int pos) {
    return data[pos];
}

string File::getName() {
    return this->name;
}

void File::setName(string newName) {
    this->name = newName;
}


int File::mkdir(string name, Filesystem fs, int iparentaddr) {
    int thisAddress = fs.getAdrSecondEmptyBlock();    
    if (thisAddress == -1) {//jika gak ada alamat buat blok filenya
        return -1;
    }
    vector<int> dataaddress;
    dataaddress.push_back(thisAddress);        
    
    //bikin inode untuk direktori ini
    int inodethisadr = Inode::consInode(&fs,Inode::DIR,dataaddress,0);        
    if(inodethisadr==-1) {//jika gak ada alamat buat blok inodenya
        return -1;
    }
    byte data[Block::BLOCK_SIZE];
    int bytecount=0;
    /**** File name stored in block ****/
    for(int i=0; i<name.length(); i++) {
        data[bytecount]=name[i];
        bytecount++;
    }
    for(int i=name.length(); i<64; i++) {
        data[bytecount]='\0';
        bytecount++;
    }    

    //link ke folder diri sendiri    
    vector<byte> thisByte = ByteUtil::intToBytes(inodethisadr);
    for (int i=0; i<thisByte.size(); i++) {
        data[bytecount] = thisByte[i];
        bytecount++;
    }

    //link ke folder parent
    vector<byte> parrentAddress = ByteUtil::intToBytes(iparentaddr);
    for (int i=0; i<parrentAddress.size(); i++) {
        data[bytecount] = parrentAddress[i];
        bytecount++;
    }        
        
    for(int i=bytecount;i<Block::BLOCK_SIZE;i++) {
        data[bytecount] = 0;
        bytecount++;
    }
    Block block(thisAddress,data);// blok data    
    fs.writeBlock(&block);    
               
    return inodethisadr;
}

int main() {
    Filesystem fs("device.txt");       
    int inodeaddr = File::mkdir("/",fs,0);
    //Filesystem::debug();
    File file(inodeaddr,fs);    
    cout<<"name : "<<file.getName()<<"\n";
    cout<<"size : "<<file.getSize()<<"\n";
    return 0;
}