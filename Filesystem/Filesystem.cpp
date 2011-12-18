#include "Filesystem.h"
#include "File.h"
#include <fstream>
#include <iostream>

Filesystem::Filesystem(string path) {       
    this->path = new char[path.size()+1];
    strcpy(this->path,path.c_str());        
    Block *superblock;
    getBlock(0,superblock);    
    byte *sizeInBytes = (*superblock).getBytes(0,4);//ambil size    
    size = ByteUtil::bytesToInt(sizeInBytes);    
    nblock = size*1000/(Block::BLOCK_SIZE);   
    bitmapStartBlockNum = 1;
    irootBlockNum = (int)ceil(nblock/Block::BLOCK_SIZE) + 2;
    dataStartBlockNum = irootBlockNum + 1;    
    delete[](sizeInBytes);
    delete(superblock);
}

Filesystem::Filesystem(const Filesystem& orig) {      
    this->path = new char[strlen(orig.getPath())];
    strcpy(path,orig.getPath());
    this->size = orig.getSize();
    this->nblock = orig.getBlockCount();
    this->bitmapStartBlockNum = orig.getBitmapStartBlockNum();
    this->irootBlockNum = orig.getIrootBlockNum();
    this->dataStartBlockNum = orig.getDataStartBlockNum();
}

Filesystem::~Filesystem() {
}

bool Filesystem::format(string path, unsigned int size) {
    byte superblock[Block::BLOCK_SIZE];
    int supblockcount=0;
    byte zero[Block::BLOCK_SIZE];
    for(int i=0;i<Block::BLOCK_SIZE;i++) {
        zero[i]=0;
    }
    if(size>Filesystem::MAX_SIZE) {
        printf("filesystem melebihi size maksimum yang diperbolehkan\n");
        return false;
    }
    int nblock = size*1000/(Block::BLOCK_SIZE);        
    int BitmapEndBlockNumber = (int)ceil(nblock/Block::BLOCK_SIZE) + 1;//nomor ujung blok bitmap     
    
    ofstream fout(path.c_str(),ios::out|ios::trunc|ios::binary);    
    if(!fout.is_open()) {
        return false;
    }
    
    //tulis size ke dalam 4 byte pertama blok 0(superblock)
    vector<byte> sizeInByte = ByteUtil::intToBytes(size);    
    for(int i=0;i<sizeInByte.size();i++) {                
        superblock[supblockcount]=sizeInByte[i];
        supblockcount++;
    }
    for(int i=0;i<Block::BLOCK_SIZE-4;i++) {        
        superblock[supblockcount] = 0;
        supblockcount++;
    }    
    fout.write((char*)superblock,Block::BLOCK_SIZE);
    
    //tulis bitmap di blok ke 1 sampai ceil(nblock/Block::BLOCK_SIZE);
    for(int i=0;i<nblock;i++) {        
        if(i<=BitmapEndBlockNumber) {            
            fout<<(byte)1;//blok superblock,bitmap
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
        fout.write((char*)zero,Block::BLOCK_SIZE);
    }
        
    fout.close();
    //buka lagi untuk mkdir root
    Filesystem fs(path);
    File::mkdir("/",fs,0);
    //buat folder root    
    return true;
}

bool Filesystem::getBlock(int number, Block *&block) {      
    ifstream fin(this->path,ios::in | ios::binary);
    
    if(!fin.is_open() || number<0) {             
        return false;
    }        
    byte data[Block::BLOCK_SIZE];        
    fin.seekg(Block::BLOCK_SIZE*number,ios::beg);                           
    fin.read((char*)data, Block::BLOCK_SIZE);
    block = new Block(number,data);        
    fin.close();
    return true;
}

bool Filesystem::writeBlock(const Block *block) {
    ofstream fout(path,ios::in | ios::binary);    
    if(!fout.is_open()) {
        return false;
    }
    fout.seekp(Block::BLOCK_SIZE*(block->number),ios::beg);    
    fout.write((char*)block->data,Block::BLOCK_SIZE);
    //nyatet di bitmap blok itu sudah terisi
    int bnumber = (block->number/Block::BLOCK_SIZE)+1;//blok bitmap blok number tersebut berada
    fout.seekp(Block::BLOCK_SIZE*bnumber+(block->number%Block::BLOCK_SIZE),ios::beg);        
    byte test = 1;
    fout.write((char *)&test,1);
    fout.close();    
    return true;
}

bool Filesystem::deleteBlock(int number) {
    ofstream fout(path,ios::in | ios::binary);    
    if(!fout.is_open()) {
        return false;
    }
    //nyatet di bitmap blok itu sudah terisi
    int bnumber = (number/Block::BLOCK_SIZE)+1;//blok bitmap blok number tersebut berada
    fout.seekp(Block::BLOCK_SIZE*bnumber+(number%Block::BLOCK_SIZE),ios::beg);        
    byte test = 0;
    fout.write((char *)&test,1);
    return true;
}

int Filesystem::getSize() const {
    return size;
}

int Filesystem::getBlockCount() const {
    return nblock;
}

int Filesystem::getBitmapStartBlockNum() const {
    return bitmapStartBlockNum;
}

int Filesystem::getIrootBlockNum() const {
    return irootBlockNum;
}

int Filesystem::getDataStartBlockNum() const {
    return dataStartBlockNum;
}

bool Filesystem::isBlockEmpty(int number) {
    ifstream fin(path,ios::in | ios::binary);
    if(!fin.is_open() || number<0) {
        return false;
    }    
    int bnumber = (number/Block::BLOCK_SIZE)+1;//blok bitmap blok number tersebut berada
    fin.seekg(Block::BLOCK_SIZE*bnumber+(number%Block::BLOCK_SIZE),ios::beg);        
    byte test;
    fin.read((char *)&test,1);
    fin.close();
    if(test==1) {//terisi
        return false;
    }    
    return true;//kosong
}

int Filesystem::getAdrEmptyBlock() {    
    int bitmapblocknum=1;    
    Block *block;    
    int blockcount=0;
           
    for(int nb=bitmapblocknum;nb<irootBlockNum;nb++) {        
        getBlock(nb,block);
        int i=0;        
        while(i<Block::BLOCK_SIZE && blockcount<nblock) {
            if((*block).getByte(i)==0) {//byte ke-i == 0 menandakan blok ke-i kosong
                return (i+((nb-bitmapblocknum)*Block::BLOCK_SIZE));
            }
            i++;
            blockcount++;
        }
        delete block;
    }
    return -1;
}

int Filesystem::getAdrSecondEmptyBlock() {
    int bitmapblocknum=1;    
    Block *block;    
    int blockcount=0;
    bool foundfirst=false; 
    
    for(int nb=bitmapblocknum;nb<irootBlockNum;nb++) {        
        getBlock(nb,block);
        int i=0;        
        while(i<Block::BLOCK_SIZE && blockcount<nblock) {
            if((*block).getByte(i)==0) {//byte ke-i == 0 menandakan blok ke-i kosong
                if(foundfirst==false) {
                    foundfirst=true;
                }else{
                    return (i+((nb-bitmapblocknum)*Block::BLOCK_SIZE));
                }
            }
            i++;
            blockcount++;
        }
        delete block;
    }
    return -1;
}

bool Filesystem::isThereNEmptyBlock(int n) {
    int bitmapblocknum=1;    
    Block *block;
    int emptycount=0;   
    int blockcount=0;
           
    for(int nb=bitmapblocknum;nb<irootBlockNum;nb++) {        
        getBlock(nb,block);
        int i=0;        
        while(i<Block::BLOCK_SIZE && blockcount<nblock) {
            if((*block).getByte(i)==0) {//byte ke-i == 0 menandakan blok ke-i kosong
                emptycount++;
            }
            if(emptycount>=n) {                
                return true;
            }
            i++;
            blockcount++;
        }
        delete block;
    }
    return false;
}

int Filesystem::getEmptyBlockCount() {
    int bitmapblocknum=1;    
    Block *block;
    int emptycount=0;   
    int blockcount=0;
     
    for(int nb=bitmapblocknum;nb<irootBlockNum;nb++) {        
        getBlock(nb,block);
        int i=0;        
        while(i<Block::BLOCK_SIZE && blockcount<nblock) {
            if((*block).getByte(i)==0) {//byte ke-i == 0 menandakan blok ke-i kosong
                emptycount++;
            }
            i++;
            blockcount++;
        }
        delete block;
    }
    return emptycount;
}

char* Filesystem::getPath() const {
    return path;
}

//int main() {
//    //tes format
//    cout<<(Filesystem::format("device.txt",(unsigned int)1024*64))<<endl;         
//    
//    //tes mount
//    Filesystem fs("device.txt");
//    printf("size : %d\n",fs.getSize());
//    printf("jumlah block : %d\n",fs.getBlockCount());
//    printf("block number bitmap : %d\n",fs.getBitmapStartBlockNum());
//    printf("block number inode root : %d\n",fs.getIrootBlockNum());
//    printf("block number data : %d\n",fs.getDataStartBlockNum());        
//    
//    printf("apakah blok 0 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(0));
//    printf("apakah blok 1 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(1));
//    printf("apakah blok 2 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(2));
//    printf("apakah blok 3 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(3));
//    printf("apakah blok 4 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(4));
//    printf("apakah blok 5 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(5));
//    printf("apakah blok 6 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(6));
//    printf("apakah blok 7 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(7));
//    printf("apakah blok 8 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(8));
//    printf("apakah blok 9 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(9));
//    printf("apakah blok 10 kosong? : %d(0==false 1==true)\n" ,fs.isBlockEmpty(10));
//    //cek getAdrEmptyBlock() 
//    printf("alamat blok kosong mana : %d\n",fs.getAdrEmptyBlock());
//    //cek getAdrSecondEmptyBlock
//    printf("alamat blok kosong kedua mana : %d\n",fs.getAdrSecondEmptyBlock());
//    
//    printf("tulis blok ke-8 sampai 4095 sebagai sudah terisi di bitmap...\n");
//    //cek writeblock()
//    Block *block;
//    fs.getBlock(fs.getBitmapStartBlockNum(),block);
//    for(int i=8;i<Block::BLOCK_SIZE;i++) {
//        block->data[i]=1;
//    }
//    fs.writeBlock(block);
//    //Filesystem::debug();
//    delete block;
//    
//    //cek getAdrEmptyBlock() 
//    printf("cek lagi blok kosong mana : %d\n",fs.getAdrEmptyBlock());
//    
//    //cek getAdrSecondEmptyBlock()
//    printf("cek lagi blok kosong kedua mana : %d\n",fs.getAdrSecondEmptyBlock());
//    
//    //cek getEmptyBlockCount()
//    printf("jumlah blok kosong : %d\n", fs.getEmptyBlockCount());
//    
//    //cek isThereNEmptyBlock() 
//    printf("cek apakah ada 11898 blok kosong : %d\n", fs.isThereNEmptyBlock(11898));
//    
//    return 0;
//}

void Filesystem::debug() {
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
    while(fin.read((char*)&tes,1)) {
        fout<<i<<" : "<<(int)tes<<endl;
        i++;
    }        
    fin.close(); 
    fout.close();
}
