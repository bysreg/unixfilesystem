#ifndef FILE_H
#define	FILE_H

#ifndef FILESYSTEM_H
#include "Filesystem.h"
#endif

#ifndef INODE_H
#include "Inode.h"
#endif

class File {    
public:    
    int inodeAddr;
    /********** CONSTRUCTOR *******/
    File(int inodeAddr, Filesystem fs);
    //name=filename, absPath=directory position, type=>true=directory, false=file
    File(const File& orig);
    virtual ~File();

    /*********** GETTER ***********/
    int getSize() const;              //get size for a file
    string getName() const;           //get local name for a file   
    //get alamat inode parent, mengembalikan 0 jika folder ini adalah root atau ini adalah file bukan folder
    int getParentInode() const;          
    byte getDataByte(int pos) const;
    vector<byte> getData() const;     
    int getType() const;
    //hanya untuk DIR
    //slot 0 menunjuk pada diri sendiri
    //slot 1 menunjuk pada parent(pada root menghasilkan angka 0, HATI-HATI!)
    int getAddress(int slot) const;
    //tambah alamat file selain diri sendiri dan parent
    void addAddress(int val,Filesystem fs);//hanya untuk DIR  

    /*****************************/
    //mengembalikan alamat blok inode file tersebut, -1 jika disk penuh
    static int mkdir(string name, Filesystem fs, int iparentaddr);
    //mengembalikan alamat blok inode file tersebut, -1 jika disk penuh
    static int mkfile(string name, Filesystem fs, int iparentaddr, byte* argdata, int argdatalen);
    //tampilin folder dan file yang berada di bawah dir
    static vector<string> ls(int iDir, Filesystem fs);
    //menampilkan working directory
    static string pwd(int iDir, Filesystem fs);
    //mengembalikan inode direktori name pada direktori iDir, mengembalikan -1 jika tidak ada atau bukan folder
    static int cd(int iDir, string name, Filesystem fs);
    //membuka file dan mengembalikan isi file pada alamat inode tesebut
    static vector<byte> cat(int inode, Filesystem fs);
    //mengambil inode file pada path tersebut(direktori maupun file), mengembalikan -1 jika tidak ada
    static int getInodeFromPath(string filepath, int curDirInode, Filesystem fs);
    //menyalin file dari filesystem sistem operasi ke virtual filesystem    
    static bool cp(string pathfile, int iDir, Filesystem fs);       
    //menyalin file dari virtual filesystem ke virtual filesystem
    static bool cp(int iFile, int iDir, Filesystem fs);
    //menyalin file dari virtual filesystem ke filesystem sistem operasi
    static bool cp(int iFile, string pathfile, Filesystem fs);
    //menghapus folder name jika ada
    static int rm(int iDir, string name, Filesystem fs);
    
    
protected:
    int size;       //file size
    string name;    //file name    
    int type;     //type, whether it is a file or a directory
    vector<byte> data;
    int iParent; // alamat inode parent    
private:
    
};

#endif	/* FILE_H */