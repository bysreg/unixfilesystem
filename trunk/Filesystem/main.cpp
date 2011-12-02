#include <cstdlib>
#include "Filesystem.h"
#include "ByteUtil.h"
#include <iostream>
#include <cstdio>
#include <fstream>

using namespace std;

void test();

int main(int argc, char** argv) {  
    //tes format
    cout<<(Filesystem::format("device.txt",(unsigned int)1024))<<endl;        
    
    //test();
    
    //tes mount
    Filesystem fs("device.txt");
    printf("size : %d\n",fs.getSize());
    printf("jumlah block : %d\n",fs.getBlockCount());
    printf("block number bitmap : %d\n",fs.getBitmapStartBlockNum());
    printf("block number inode root : %d\n",fs.getIrootBlockNum());
    printf("block number data : %d\n",fs.getDataStartBlockNum());        
    
    printf("apakah blok 0 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(0));
    printf("apakah blok 1 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(1));
    printf("apakah blok 2 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(2));
    printf("apakah blok 3 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(3));
    printf("apakah blok 4 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(4));
    printf("apakah blok 5 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(5));
    printf("apakah blok 6 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(6));
    return 0;
}

void test() {
    string pathin = "device.txt";
    ifstream fin(pathin.c_str(),ios::in | ios::binary);
    string path = "out.txt";
    ofstream fout(path.c_str(),ios::out | ios::trunc);
    if(!fin.is_open()) {
        printf("gagal tesst\n");
        return;
    }    
    if(!fout.is_open()) {
        printf("gagal tesst\n");
        return;
    }    
    byte tes;
    int i=0;
    while(fin>>tes) {
        fout<<i<<" : "<<(int)tes<<endl;
        i++;
    }        
    fin.close();    
}
