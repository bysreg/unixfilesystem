#include <cstdlib>
#include "Filesystem.h"
#include "ByteUtil.h"
#include <iostream>
#include <cstdio>

using namespace std;

int main(int argc, char** argv) {  
    //tes format
    cout<<(Filesystem::format("device.txt",(unsigned int)1024))<<endl;        
    
    //tes mount
    Filesystem fs("device.txt");
    printf("size : %d\n",fs.getSize());
    printf("jumlah block : %d\n",fs.getBlockCount());
    printf("block number bitmap : %d\n",fs.getBitmapStartBlockNum());
    printf("block number inode root : %d\n",fs.getIrootBlockNum());
    printf("block number data : %d\n",fs.getDataStartBlockNum());    
    Block* bitmap;
    fs.getBlock(1,bitmap);
    printf("apakah superblock terisi? : %d\n",bitmap->getByte(0));
    printf("apakah block 1 terisi? :  %d\n",bitmap->getByte(1));
    printf("apakah block 2 terisi? :  %d\n",bitmap->getByte(2));
    printf("apakah block 3 terisi? :  %d\n",bitmap->getByte(3));
    printf("apakah block 4 terisi? :  %d\n",bitmap->getByte(4));
    printf("apakah block 5 terisi? :  %d\n",bitmap->getByte(5));    
    printf("apakah block 6 terisi? :  %d\n",bitmap->getByte(6));    
    return 0;
}

