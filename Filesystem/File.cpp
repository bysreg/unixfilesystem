#include <stdlib.h>
#include "File.h"
#include <iostream>
#include <fstream>
#include <cstdio>

using namespace std;

File::File(int iAddr, Filesystem fs) {
    Inode inode(iAddr, fs);
    int slot = 0, dataAddr;
    Block* block;
    this->type = inode.getType();
    if (this->type == Inode::FILE) {
        int bytecount = 0;
        this->size = inode.getFileSize();
        //ambil data
        while ((dataAddr = inode.getDataAddress(slot)) != -1) {
            fs.getBlock(dataAddr, block);
            for (int i = 0; i < Block::BLOCK_SIZE; i++) {
                if (slot == 0 && i < 64) {//skip bagian nama
                    continue;
                }
                data.push_back(block->getByte(i));
                bytecount++;
                if (bytecount >= inode.getFileSize()) {
                    break;
                }
            }
            delete block;
            slot++;
        }
        //ambil nama
        fs.getBlock(inode.getDataAddress(0), block);
        char c_name[64];
        char c;
        int i = 0;
        while ((c = block->getByte(i)) != '\0') {
            c_name[i] = c;
            i++;
        }
        c_name[i] = '\0';
        this->name = c_name;
        //ambil parent inode
        this->iParent = 0;
        delete block;
        return;
    }
    //jika DIR
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
    int i = 0;
    //ambil nama
    while ((c = data[i]) != '\0') {
        c_name[i] = c;
        i++;
    }
    c_name[i] = '\0';
    this->name = c_name;

    //ambil parent inode    
    byte b_iParent[4];
    for (int i = 68; i <= 71; i++) {
        b_iParent[i - 68] = this->data[i];
    }
    this->iParent = ByteUtil::bytesToInt(b_iParent);
}

File::File(const File& orig) {
    this->size = orig.getSize();
    this->name = orig.getName();
    this->type = orig.getType();
    for (int i = 0; i < orig.getSize(); i++) {
        this->data.push_back(orig.getDataByte(i));
    }
}

File::~File() {
}

int File::getSize() const {
    return this->size;
}

int File::getType() const {
    return this->type;
}

byte File::getDataByte(int pos) const {
    return data[pos];
}

vector<byte> File::getData() const {
    return data;
}

string File::getName() const {
    return this->name;
}

int File::getParentInode() const {
    return this->iParent;
}

int File::mkdir(string name, Filesystem fs, int iparentaddr) {
    int thisAddress = fs.getAdrSecondEmptyBlock();
    if (thisAddress == -1) {//jika gak ada alamat buat blok filenya
        return -1;
    }
    vector<int> dataaddress;
    dataaddress.push_back(thisAddress);

    //bikin inode untuk direktori ini
    int inodethisadr = Inode::consInode(&fs, Inode::DIR, dataaddress, 0);
    if (inodethisadr == -1) {//jika gak ada alamat buat blok inodenya
        return -1;
    }
    byte data[Block::BLOCK_SIZE];
    int bytecount = 0;
    /**** 64 byte pertama untuk nama ****/
    for (int i = 0; i < name.length(); i++) {
        data[bytecount] = name[i];
        bytecount++;
    }
    for (int i = name.length(); i < 64; i++) {
        data[bytecount] = '\0';
        bytecount++;
    }

    //kemudian 4 byte untuk link ke folder diri sendiri(byte 64-67)    
    vector<byte> thisByte = ByteUtil::intToBytes(inodethisadr);
    for (int i = 0; i < thisByte.size(); i++) {
        data[bytecount] = thisByte[i];
        bytecount++;
    }

    //kemudian 4 byte untuk link ke folder parent(byte 68-71)    
    vector<byte> parrentAddress = ByteUtil::intToBytes(iparentaddr);
    for (int i = 0; i < parrentAddress.size(); i++) {
        data[bytecount] = parrentAddress[i];
        bytecount++;
    }

    for (int i = bytecount; i < Block::BLOCK_SIZE; i++) {
        data[bytecount] = 0;
        bytecount++;
    }
    Block block(thisAddress, data); // blok data    
    fs.writeBlock(&block);

    if (iparentaddr != 0) {
        File parent(iparentaddr, fs);
        parent.addAddress(inodethisadr, fs);
    }

    return inodethisadr;
}

int File::mkfile(string name, Filesystem fs, int iparentaddr, byte* argdata, int argdatalen) {
    int thisAddress = fs.getAdrSecondEmptyBlock();
    if (thisAddress == -1) {//jika gak ada alamat buat blok filenya
        return -1;
    }
    vector<int> dataaddress;
    dataaddress.push_back(thisAddress);

    //bikin inode untuk file ini dengan size argdatalen
    int inodethisadr = Inode::consInode(&fs, Inode::FILE, dataaddress, argdatalen);
    if (inodethisadr == -1) {//jika gak ada alamat buat blok inodenya
        return -1;
    }
    byte data[Block::BLOCK_SIZE];
    int bytecount = 0;
    /**** 64 byte pertama untuk nama ****/
    for (int i = 0; i < name.length(); i++) {
        data[bytecount] = name[i];
        bytecount++;
    }
    for (int i = name.length(); i < 64; i++) {
        data[bytecount] = '\0';
        bytecount++;
    }
    //byte berikutnya untuk data
    for (int i = 0; i < argdatalen; i++) {
        data[bytecount] = argdata[i];
        bytecount++;
    }
    //isi byte sisa dengan 0
    while (bytecount < Block::BLOCK_SIZE) {
        data[bytecount] = 0;
        bytecount++;
    }
    //tulis block data file ini ke fs
    Block block(thisAddress, data); // blok data    
    fs.writeBlock(&block);
    //tambah entri inode file ini ke direktori iparentaddr
    if (iparentaddr != 0) {
        File parent(iparentaddr, fs);
        parent.addAddress(inodethisadr, fs);
    }
    return inodethisadr;
}

vector<string> File::ls(int iDirAddr, Filesystem fs) {
    vector<string> ret;
    //akses inode iDir    
    File dir(iDirAddr, fs);
    int addr; //alamat inode direktori/file yang berada dalam direktori
    int slot = 0;
    while (true) {
        if (iDirAddr == fs.getIrootBlockNum() && slot == 1) {
            slot++;
            continue;
        }
        addr = dir.getAddress(slot);
        if (addr != 0) {
            File file(addr, fs);
            ret.push_back(file.getName());
        } else {
            break;
        }
        slot++;
    }
    return ret;
}

string File::pwd(int iDir, Filesystem fs) {
    string ret = "";
    bool first = false;
    int par;
    File *file = new File(iDir, fs);
    while (true) {
        //cout<<file->getName()<<" "<<file->getParentInode()<<endl;
        if (first == false) {
            first = true;
            ret = file->getName();
        } else {
            ret = file->getName() + "/" + ret;
        }
        if (file->getParentInode() == 0) {
            break;
        }
        delete file;
        file = new File(file->getParentInode(), fs);
    }
    return ret;
}

int File::cd(int iDirAddr, string name, Filesystem fs) {
    //akses inode iDir    
    File dir(iDirAddr, fs);
    if (name == ".." && iDirAddr != fs.getIrootBlockNum()) {
        return dir.getParentInode();
    } else if (name == ".") {
        return iDirAddr;
    }
    int addr; //alamat inode direktori/file yang berada dalam direktori
    int slot = 0;
    while (true) {
        if (iDirAddr == fs.getIrootBlockNum() && slot == 1) {
            slot++;
            continue;
        }
        addr = dir.getAddress(slot);
        if (addr != 0) {
            File file(addr, fs);
            if (file.getName() == name && file.getType() == Inode::DIR) {
                return file.getAddress(0); //alamat inode folder ini sendiri
            }
        } else {
            break;
        }
        slot++;
    }
    return -1; //tidak ketemu
}

vector<byte> File::cat(int iFile, Filesystem fs) {
    vector<byte> ret;
    File file(iFile, fs);
    return file.getData();
}

int File::getInodeFromPath(string filepath, int curDirInode, Filesystem fs) {
    int iaddress;
    int ret;
    if (filepath[0] == '/') {
        //jika karakter pertama '/' berarti yang dipake absolut(search dari root)
        ret = fs.getIrootBlockNum();
    } else {
        ret = curDirInode;
    }
    vector<string> dirlist;
    int i, j, awal;
    for (i = 0; i < filepath.length(); i++) {
        if (filepath[i] != '/') {
            awal = i;
            j = 0;
            while ((filepath[i] != '/') && (i < filepath.length())) {
                j++;
                i++;
            }
            dirlist.push_back(filepath.substr(awal, j));
        }
    }
    for (int i = 0; i < dirlist.size(); i++) {        
        if ((iaddress = cd(ret, dirlist[i], fs)) == -1) {
            //mungkin yang ditunjuk oleh ret adalah file(cd hanya bisa ganti folder)
            File dir(ret,fs);
            int slot=0,addr=0;
            while (true) {                
                addr = dir.getAddress(slot);
                if (addr != 0) {
                    File file(addr, fs);
                    if (file.getName() == dirlist[i] && file.getType() == Inode::FILE) {
                        return addr;
                    }
                } else {
                    break;
                }
                slot++;
            }
            return -1;
        }
        ret = iaddress;
    }
    return ret;
}

bool File::cp(string pathfile, int iDestDir, Filesystem fs) {
    if(iDestDir<=0) {
        return false;
    }
    //buka file dari OS
    ifstream fin(pathfile.c_str(), ios::in | ios::binary);
    int flen = 0;

    if (!fin.is_open()) {
        return false;
    }

    //dapetin panjang file
    fin.seekg(0, ios::end);
    flen = fin.tellg();
    fin.seekg(0, ios::beg);

    byte data[flen];
    fin.read((char*) data, flen);
    fin.close();

    vector<string> dirlist;
    int i, j, awal;
    for (i = 0; i < pathfile.length(); i++) {
        if (pathfile[i] != '/') {
            awal = i;
            j = 0;
            while ((pathfile[i] != '/') && (i < pathfile.length())) {
                j++;
                i++;
            }
            dirlist.push_back(pathfile.substr(awal, j));
        }
    }
    //data sudah terisi dengan isi file, sekarang copy isinya ke virtual FS
    mkfile(dirlist.back(), fs, iDestDir, data, flen);
    return true;
}

//FIXME : masih belom selesai
int File::rm(int iDirAddr, string name, Filesystem fs) {
    //akses inode iDir    
    File dir(iDirAddr, fs);
    if (name == ".." && iDirAddr != fs.getIrootBlockNum()) {
        return dir.getParentInode();
    } else if (name == ".") {
        return iDirAddr;
    }
    int addr; //alamat inode direktori/file yang berada dalam direktori
    int slot = 0;
    while (true) {
        if (iDirAddr == fs.getIrootBlockNum() && slot == 1) {
            slot++;
            continue;
        }
        addr = dir.getAddress(slot);
        if (addr != 0) {
            File file(addr, fs);
            if (file.getName() == name && file.getType()) {

                return file.getAddress(0); //alamat inode folder ini sendiri
            }
        } else {
            break;
        }
        slot++;
    }
    return -1; //tidak ketemu
}

int File::getAddress(int slot) const {
    int base = 64;
    byte b_addr[4];
    int bytecount = 0;
    for (int i = slot * 4 + base; i < slot * 4 + base + 4; i++) {
        b_addr[bytecount] = data[i];
        bytecount++;
    }
    return ByteUtil::bytesToInt(b_addr);
}

void File::addAddress(int val, Filesystem fs) {
    int slot = 2;
    while (getAddress(slot) != 0) {
        slot++;
    }
    vector<byte> b_val = ByteUtil::intToBytes(val);
    for (int i = 0; i < 4; i++) {
        data[slot * 4 + 64 + i] = b_val[i];
    }
    Inode self(getAddress(0), fs);
    Block block(self.getDataAddress(0), &data);
    fs.writeBlock(&block);
}

int main() {
    Filesystem::format("device.txt", (unsigned int) 1024 * 17);
    Filesystem fs("device.txt");
    int inodeaddr = fs.getIrootBlockNum();
    int usrinodeaddr = File::mkdir("usr", fs, inodeaddr);
    File::mkdir("a", fs, inodeaddr);
    File::mkdir("b", fs, inodeaddr);
    printf("inode addr %d \n", inodeaddr);
    File root(inodeaddr, fs);
    File usr(usrinodeaddr, fs);
    printf("root %d parent : %d\n", inodeaddr, root.getParentInode());
    printf("usr : %d parent : %d\n", usrinodeaddr, usr.getParentInode());
    //Filesystem::debug();

    cout << "name : " << root.getName() << "\n";
    cout << "size : " << root.getSize() << "\n";
    cout << "name : " << usr.getName() << "\n";
    cout << "size : " << usr.getSize() << "\n";

    printf("testing getAddress\n");
    printf("root 0 : %d\n", root.getAddress(0));
    printf("root 1 : %d\n", root.getAddress(1));
    printf("root 2 : %d\n", root.getAddress(2));
    printf("usr 0 : %d\n", usr.getAddress(0));
    printf("usr 1 : %d\n", usr.getAddress(1));
    printf("usr 2 : %d\n", usr.getAddress(2));
    File::mkdir("agafw", fs, 5);
    File::mkdir("agafadww", fs, 5);
    printf("testing ls\n");
    vector<string> retLs = File::ls(5, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    int a = File::mkdir("ea", fs, 5);
    int b = File::mkdir("uhuy", fs, a);
    int c = File::mkdir("hilman", fs, b);
    int d = File::mkdir("beyri", fs, c);

    printf("\ntesting pwd\n");
    cout << "path usr : " << File::pwd(d, fs) << endl;

    printf("\ncd \n");
    cout << "direktori hilman pada dir beyri " << File::cd(5, "..", fs) << " " << "direktori beyri adalah : " << c << endl;

    printf("\ntesting mkfile : \n");
    byte testdatafile[10];
    testdatafile[0] = 'h';
    testdatafile[1] = 'a';
    testdatafile[2] = 'l';
    testdatafile[3] = 'o';
    testdatafile[4] = ' ';
    testdatafile[5] = 'h';
    testdatafile[6] = 'a';
    testdatafile[7] = 'l';
    testdatafile[8] = 'o';
    testdatafile[9] = '\0';
    int itestfile = File::mkfile("test1.txt", fs, 5, testdatafile, 10);
    printf("inode test file = %d\n", itestfile);
    printf("dir usr sekarang : \n");
    retLs = File::ls(5, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    printf("\ntesting cat pada test file : \n");
    vector<byte> retcat = File::cat(itestfile, fs);
    for (int i = 0; i < retcat.size(); i++) {
        if (i % 4 == 0) {//print 4 byte tiap baris
            cout << endl << i << ". ";
        }
        cout << (char) retcat[i] << " ";
    }
    printf("\n\ntesting cp : \n");
    File::cp("tes/wafaw/afwafa", 5, fs);
    printf("dir usr sekarang : \n");
    retLs = File::ls(5, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    printf("cat isi afwafa :");
    retcat = File::cat(itestfile, fs);
    for (int i = 0; i < retcat.size(); i++) {
        if (i % 4 == 0) {//print 4 byte tiap baris
            cout << endl << i << ". ";
        }
        cout << (char) retcat[i] << " ";
    }
    printf("\n");
    printf("testing getInodeFromPath : \n");
    printf("inode afwafa(absolut) : %d\n", File::getInodeFromPath("/usr/afwafa", 5, fs));
    printf("inode afwafa(relatif) : %d\n", File::getInodeFromPath("afwafa", 5, fs));//relatif
    File::cp("tes/wafaw/haha", File::getInodeFromPath(".",d,fs), fs);
    retLs = File::ls(d, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    return 0;
}