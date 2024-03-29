#include <stdlib.h>
#include "File.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cmath>
#include <sys/stat.h>
#include <dirent.h>

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
    //jumlah blok dibutuhkan = 1(utk inode file tsb) + ceil(argdatalen/BLOCK_SIZE) 
    if (!fs.isThereNEmptyBlock((double) argdatalen / Block::BLOCK_SIZE + 1)) {
        return -1;
    }

    //dataaddress menampung alamat-alamat blok dimana data file ini disimpan
    vector<int> dataaddress;

    /***mulai pembuatan blok-blok data file***/
    byte data[Block::BLOCK_SIZE];
    int bytecount = 0;
    // 64 byte pertama untuk nama
    for (int i = 0; i < name.length(); i++) {
        data[bytecount] = name[i];
        bytecount++;
    }
    for (int i = name.length(); i < 64; i++) {
        data[bytecount] = '\0';
        bytecount++;
    }
    int datacount = 0;
    //byte-byte berikutnya untuk data(simpan di beberapa blok jika perlu)
    for (int i = 0; i < ceil((double) argdatalen / Block::BLOCK_SIZE); i++) {
        int thisAddress = fs.getAdrEmptyBlock();
        while (bytecount < Block::BLOCK_SIZE && datacount < argdatalen) {
            data[bytecount] = argdata[datacount];
            bytecount++;
            datacount++;
        }
        //jika blok yang sedang diisi belum penuh, penuhi dengan angka 0
        while (bytecount < Block::BLOCK_SIZE) {
            data[bytecount] = 0;
            bytecount++;
        }
        //tulis block data file ini ke fs
        Block block(thisAddress, data);
        fs.writeBlock(&block);
        dataaddress.push_back(thisAddress);
        bytecount = 0;
    }

    //bikin inode untuk file ini dengan size argdatalen
    int inodethisadr = Inode::consInode(&fs, Inode::FILE, dataaddress, argdatalen);
    if (inodethisadr == -1) {//jika gak ada alamat buat blok inodenya        
        return -1;
    }

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
        if (addr != 0 && addr != MARK_RECYCLE) {
            File file(addr, fs);
            ret.push_back(file.getName());
        } else if (addr == 0) {
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
    if (iDirAddr <= 0 || iDirAddr == MARK_RECYCLE) {
        return -1;
    }
    //akses inode iDir    
    File dir(iDirAddr, fs);
    if (dir.getType() == Inode::FILE) {//cek iDirAddrnya apakah file
        return -1;
    }
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
        if (addr != 0 && addr != MARK_RECYCLE) {
            File file(addr, fs);
            if (file.getName() == name && file.getType() == Inode::DIR) {
                return file.getAddress(0); //alamat inode folder ini sendiri
            }
        } else if (addr == 0) {
            break;
        }
        slot++;
    }
    return -1; //tidak ketemu
}

vector<byte> File::cat(int iFile, Filesystem fs) {
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
            File dir(ret, fs);
            int slot = 0, addr = 0;
            while (true) {
                if (curDirInode == fs.getIrootBlockNum() && slot == 1) {
                    slot++;
                    continue;
                }
                addr = dir.getAddress(slot);
                if (addr != 0 && addr != MARK_RECYCLE) {
                    File file(addr, fs);
                    if (file.getName() == dirlist[i] && file.getType() == Inode::FILE) {
                        return addr;
                    }
                } else if (addr == 0) {
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

int File::getInodeParentFromPath(string filepath,int curDirInode, Filesystem fs) {
    int iaddress;
    int ret;
    if (filepath[0] == '/') {
        //jika karakter pertama '/' berarti yang dipake absolut(search dari root)
        ret = fs.getIrootBlockNum();
    } else {
        ret = curDirInode;
    }    
    if(File::getInodeFromPath(filepath,curDirInode,fs)==-1) {//file yang mau dicari parent nya gak ada
        return -1;
    }
    vector<string> dirlist = File::parsePath(filepath);            
    for (int i = 0; i < (((int)dirlist.size())-1); i++) {//file terakhir gak dilacak        
        cout<<dirlist[i]<<endl;
        if ((iaddress = cd(ret, dirlist[i], fs)) == -1) {
            //mungkin yang ditunjuk oleh ret adalah file(cd hanya bisa ganti folder)
            File dir(ret, fs);
            int slot = 0, addr = 0;
            while (true) {
                if (curDirInode == fs.getIrootBlockNum() && slot == 1) {
                    slot++;
                    continue;
                }
                addr = dir.getAddress(slot);
                if (addr != 0 && addr != MARK_RECYCLE) {
                    File file(addr, fs);
                    if (file.getName() == dirlist[i] && file.getType() == Inode::FILE) {
                        return addr;
                    }
                } else if (addr == 0) {
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

bool File::cp_folder(string pathfile, int iDestDir, Filesystem fs) {
    int dir_count = 0;
    struct dirent* dent;
    DIR* srcdir = opendir(pathfile.c_str());
    if (srcdir == NULL) {        
        return false;
    }
    //buat direktori tersebut juga di virtual fs dibawah folder iDestDir
    vector<string> dirlist = File::parsePath(pathfile);
    string dirname = dirlist.back();        
    int faddress = File::mkdir(dirname,fs,iDestDir);//faddress adalah alamat inode folder ini di virtual FS
    if(faddress==-1) {
        return false;
    }
    //cari folder/file anak folder ini
    while ((dent = readdir(srcdir)) != NULL) {
        struct stat st;

        if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0)
            continue;

        if (fstatat(dirfd(srcdir), dent->d_name, &st, 0) < 0) {
            perror(dent->d_name);
            continue;
        }
        char nextlook[100];
        strcpy(nextlook, pathfile.c_str());
        strcat(nextlook, "/");
        strcat(nextlook, dent->d_name);
        if (S_ISDIR(st.st_mode)) {
            dir_count++;
            printf("direktori %s\n", dent->d_name);                       
            cp_folder(nextlook, faddress, fs);
        } else if (S_ISREG(st.st_mode)) {
            printf("file %s\n", dent->d_name);
            cp(nextlook, faddress, fs);
        }
    }
    closedir(srcdir);
    return true;
}

bool File::cp(string pathfile, int iDestDir, Filesystem fs) {
    if (iDestDir <= 0) {
        return false;
    }

    int status;
    struct stat st_buf;

    //cek apakah pathfile adalah folder atau file
    status = stat(pathfile.c_str(), &st_buf);
    if (status != 0) {
        return false; //error
    }

    if (S_ISDIR(st_buf.st_mode)) {//rekurens
        return cp_folder(pathfile, iDestDir, fs);
    } else if (S_ISREG(st_buf.st_mode)) {//basis
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

        vector<string> dirlist = File::parsePath(pathfile);        
        //data sudah terisi dengan isi file, sekarang copy isinya ke virtual FS
        int iserr = mkfile(dirlist.back(), fs, iDestDir, data, flen);
        if (iserr == -1) {
            return false;
        }
        return true;
    }
    return false; //error jika bukan file ataupun folder
}

bool File::cp(int iFile, int iDir, Filesystem fs) {
    if (iFile <= 0 && iDir <= 0) {
        return false;
    }

    File source(iFile, fs);
    if(source.getType()==Inode::FILE) {
        int iserr = File::mkfile(source.getName(), fs, iDir, &(source.getData()[0]), source.getSize());
        if (iserr == -1) {
            return false;
        }
        return true;
    }else if(source.getType()==Inode::DIR) {
        //1. buat folder salinan di tujuan
        int faddress = File::mkdir(source.getName(), fs, iDir);
        //2. copy semua anak di folder sumber ke folder faddress        
        int address = 0;
        int slot = 2;
        while ((address = source.getAddress(slot)) != 0) {
            if (address != MARK_RECYCLE) {
                cp(address, faddress, fs);
            }
            slot++;
        }
        return true;
    }
    return false;
}

bool File::cp(int iFile, string dirDest, Filesystem fs) {
    if (iFile <= 0) {
        return false;
    }

    int status;
    struct stat st_buf;

    //cek apakah pathfile adalah folder(jika bukan, return false)
    status = stat(dirDest.c_str(), &st_buf);
    if (status != 0) {
        return false; //error
    }

    if (!S_ISDIR(st_buf.st_mode)) {
        return false; //error
    }

    //buka file yang ingin dicopy
    File source(iFile, fs);
    if(source.getType()==Inode::FILE) {//basis    
        //buka folder tujuan copy
        string destpathfile = dirDest + "/" + source.getName();
        ofstream fout(destpathfile.c_str(), ios::binary);
        fout.write((char*) (&(source.getData()[0])), source.getSize());
        fout.close();
        return true;
    }else if(source.getType()==Inode::DIR) {//rekursif        
        //1. buat folder di tujuan
        if(Conflict::LINUXmkdir((dirDest+"/"+source.getName()).c_str(),0777)==-1) {//INI BUKAN mkdir program ini, tapi ini adalah mkdir bawaan linux
            return false;
        }else{
            //2. iterasi folder anaknya
            int address = 0;
            int slot = 2;
            while ((address = source.getAddress(slot)) != 0) {
                if (address != MARK_RECYCLE) {
                    cp(address, (dirDest+"/"+source.getName()).c_str(), fs);
                }
                slot++;
            }
            return true;
        }
        return false;
    }
}

bool File::rm(string pathfile, int iCurDir, Filesystem fs) {
    int iFile = File::getInodeFromPath(pathfile,iCurDir,fs);
    int iParDir = File::getInodeParentFromPath(pathfile,iCurDir,fs);
    return rm(iFile,iParDir,fs);
}

bool File::rm(int iFile, int iParDir, Filesystem fs) {
    if (iFile <= 0 || iParDir <= 0 || iFile == iParDir) {
        return false;
    }
    //akses inode iFile  
    Inode inodeFile(iFile, fs);
    if (inodeFile.getType() == Inode::DIR) {
        //1. delete semua anak2nya(mulai dari slot 2)
        File dir(iFile, fs);
        int address = 0;
        int slot = 2;
        while ((address = dir.getAddress(slot)) != 0) {//iterasi tiap anak
            if (address != MARK_RECYCLE) {
                File::rm(address, iFile, fs);
            }
            slot++;
        }
        //2. setelah anak-anak didelete, apus data file folder diri sendiri        
        fs.deleteBlock(inodeFile.getDataAddress(0));
        //3. apus inode diri sendiri
        fs.deleteBlock(iFile);
        //4. kemudian isi alamat direktori yang memuat alamat inode direktori ini dengan MARK_RECYCLE
        File curDir(iParDir, fs);
        slot = 2; //slot mulai dari setelah diri sendiri dan parent
        while (curDir.getAddress(slot) != iFile && curDir.getAddress(slot) != 0) {
            slot++;
        }
        if (curDir.getAddress(slot) == iFile) {//ketemu
            curDir.delAddress(slot, fs);            
            return true;
        } else {
            printf("KETEMU COK\n");
            return false;
        }
    } else if (inodeFile.getType() == Inode::FILE) {
        //1. delete semua blok data file
        int slot = 0;
        int dataaddress = 0;
        while ((dataaddress = inodeFile.getDataAddress(slot)) != -1) {
            fs.deleteBlock(dataaddress);
            slot++;
        }
        //2. delete blok inode file ini
        fs.deleteBlock(iFile);
        //3. isi alamat direktori yang memuat alamat inode file ini dengan MARK_RECYCLE
        File dir(iParDir, fs);
        slot = 2; //slot mulai dari setelah diri sendiri dan parent
        while (dir.getAddress(slot) != iFile && dir.getAddress(slot) != 0) {
            slot++;
        }
        if (dir.getAddress(slot) != 0) {
            dir.delAddress(slot, fs);
            return true;
        } else {
            return false;
        }
    }
    return false; //tidak ketemu
}

bool File::mv(string source, string dest, int iCurDir, Filesystem fs) {
    if(source[0]=='@') {//pindahin dari OS ke virtual FS
        string psource = source.substr(1);
        cp(psource,File::getInodeFromPath(dest,iCurDir,fs),fs);
        File::removedirectoryrecursively(psource.c_str());
    }else if(dest[0]=='@') {//pindahin dari virtual FS ke OS
        string pdest = dest.substr(1);
        cp(File::getInodeFromPath(source,iCurDir,fs),pdest,fs);
        rm(File::getInodeFromPath(source,iCurDir,fs),File::getInodeParentFromPath(source,iCurDir,fs),fs);
    }else if(source[0]!='@' && dest[0]!='@') {//pindahin dari virtual FS ke virtual FS sendiri
        cp(File::getInodeFromPath(source,iCurDir,fs),File::getInodeFromPath(dest,iCurDir,fs),fs);
        rm(File::getInodeFromPath(source,iCurDir,fs),iCurDir,fs);
    }
    return true;
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
    while (getAddress(slot) != 0 && getAddress(slot) != MARK_RECYCLE) {
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

void File::delAddress(int slot, Filesystem fs) {
    vector<byte> b_val = ByteUtil::intToBytes(MARK_RECYCLE);
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
    printf("jumlah blok kosong : %d\n", fs.getEmptyBlockCount());
    int inodeaddr = fs.getIrootBlockNum();
    int usrinodeaddr = File::mkdir("usr", fs, inodeaddr);
    File::mkdir("a", fs, inodeaddr);
    int iB = File::mkdir("b", fs, inodeaddr);
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

    printf("\n\ntesting cp afwafa ke fs: \n");
    File::cp("tes/wafaw/afwafa", 5, fs);
    printf("dir usr sekarang : \n");
    retLs = File::ls(5, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    printf("cat isi afwafa :");
    retcat = File::cat(File::getInodeFromPath("afwafa", 5, fs), fs);
    for (int i = 0; i < retcat.size(); i++) {
        if (i % 4 == 0) {//print 4 byte tiap baris
            cout << endl << i << ". ";
        }
        cout << (char) retcat[i] << " ";
    }
    printf("\n");

    printf("testing getInodeFromPath : \n");
    printf("inode afwafa(absolut) : %d\n", File::getInodeFromPath("/usr/afwafa", 5, fs));
    printf("inode afwafa(relatif) : %d\n", File::getInodeFromPath("afwafa", 5, fs)); //relatif
    File::cp("tes/wafaw/haha", File::getInodeFromPath(".", d, fs), fs);
    retLs = File::ls(d, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    printf("isi haha : \n");
    retcat = File::cat(File::getInodeFromPath("haha", d, fs), fs);
    for (int i = 0; i < retcat.size(); i++) {
        if (i % 4 == 0) {//print 4 byte tiap baris
            cout << endl << i << ". ";
        }
        cout << (char) retcat[i] << " ";
    }

    printf("\ntesting cp dengan file lebih dari 4032 byte");
    File::cp("tes/wafaw/beginning_linux_programming_2nd_edition.pdf", 5, fs);
    printf("dir usr sekarang : \n");
    retLs = File::ls(5, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    printf("alamat inode nya  : %d\n", File::getInodeFromPath("beginning_linux_programming_2nd_edition.pdf", 5, fs));

    printf("\ntesting cp dari virtual fs ke dirinya sendiri cuman beda folder");
    bool ea = File::cp(File::getInodeFromPath("beginning_linux_programming_2nd_edition.pdf", 5, fs), fs.getIrootBlockNum(), fs);
    printf("\ncopy berhasil ? : %d\n", ea);
    printf("dir root sekarang : \n");
    retLs = File::ls(fs.getIrootBlockNum(), fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    printf("hasil copy %d \n", File::getInodeFromPath("beginning_linux_programming_2nd_edition.pdf", fs.getIrootBlockNum(), fs));
    printf("\ntes kedua, copy folder usr ke bawah folder ");

    printf("\ntesting cp dari virtual fs ke luar fs(ke OS asli)\n");
    File::cp(File::getInodeFromPath("beginning_linux_programming_2nd_edition.pdf", fs.getIrootBlockNum(), fs), "..", fs);

    printf("\ntesting remove file test1.txt di folder usr\n");
    printf("sebelum ngapus blok kosong ada %d\n", fs.getEmptyBlockCount());
    File::rm(File::getInodeFromPath("test1.txt", 5, fs), 5, fs);
    printf("dir usr sekarang : \n");
    retLs = File::ls(5, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    printf("setelah ngapus blok kosong ada %d\n", fs.getEmptyBlockCount());

    printf("\ntesting remove folder ea yang ada pada folder usr\n");
    File::rm(File::getInodeFromPath("ea", 5, fs), 5, fs);
    printf("dir usr sekarang : \n");
    retLs = File::ls(5, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    printf("jumlah blok kosong sekarang : %d\n", fs.getEmptyBlockCount());
    printf("inode folder ea : %d\n", File::getInodeFromPath("aaaa", 5, fs));
    
    printf("\ntesting copy folder mantab dari luar secara rekursif ke dalam folder usr di virtual file system\n");
    File::cp("mantab",5,fs);
    printf("dir usr sekarang : \n");
    retLs = File::ls(5, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }    
    printf("dir mantab sekarang : \n");
    retLs = File::ls(File::getInodeFromPath("mantab",5,fs), fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }       
    
    printf("\ntesting copy folder jaya yang ada di //usr/mantab ke root\n");
    File::cp(File::getInodeFromPath("/usr/mantab/jaya",5,fs),fs.getIrootBlockNum(),fs);
    printf("dir root sekarang : \n");
    retLs = File::ls(3, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    
    printf("\ntesting copy folder usr yang ada di virtual FS ke OS"); 
    File::cp(5,"testout",fs);
    
    printf("\ntesting mv folder mantab di dalam usr ke folder root");
    File::mv("/usr/mantab","/",5,fs);
    printf("dir root sekarang : \n");
    retLs = File::ls(3, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    printf("dir usr sekarang : \n");
    retLs = File::ls(5, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    
    printf("\ntesting mv folder jaya yang sekarang di root ke OS\n");
    printf("dir root sebelum move : \n");
    retLs = File::ls(3, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }
    File::mv("/jaya","@.",5,fs);    
    printf("dir root sekarang : \n");
    retLs = File::ls(3, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }    
    
    printf("\n testing move dari OS ke virtual FS\n");
    File::mv("@nuketest","/",5,fs);
    printf("dir root sekarang : \n");
    retLs = File::ls(3, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }   
    
    printf("\ntesting getInodeParentFromPath\n");
    printf("%d\n",File::getInodeParentFromPath("usr/afwafa",3,fs));
    printf("%d\n",File::getInodeFromPath("uaaa",3,fs));
    
    printf("\ntesting rm pake string(ngapus folder b)\n");
    File::rm("/usr/afwafa",5,fs);
    printf("dir usr sekarang : \n");
    retLs = File::ls(5, fs);
    for (int i = 0; i < retLs.size(); i++) {
        cout << "dir " << i << " : " << retLs[i] << endl;
    }   
    return 0;
}

vector<string> File::parsePath(string filepath) {
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
    return dirlist;
}

int File::removedirectoryrecursively(const char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    char path[PATH_MAX];
    
    dir = opendir(dirname);
    if (dir == NULL) {
        perror("Error opendir()");
        return 0;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
            snprintf(path, (size_t) PATH_MAX, "%s/%s", dirname, entry->d_name);
            if (entry->d_type == DT_DIR) {
                removedirectoryrecursively(path);
            }

            /*
             * Yang sekarang dipakai hanya simulasi
             * Untuk beneran pake : remove*STUB*(path);
             * info : man 3 remove                         
             */
            printf("(simulasi) Menghapus: %s\n", path);            
        }

    }
    closedir(dir);

    //direktori ini sudah kosong, sekarang delete folder ini sendiri
    printf("(simulasi) Menghapus: %s\n", dirname);

    return 1;
}