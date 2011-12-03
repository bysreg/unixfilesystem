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
    //string getAbsoluteName();   //get absoulute name from root
    int getParentInode() const;           //get alamat inode parent, mengembalikan -1 jika folder ini adalah root
    byte getDataByte(int pos) const;
    void getOtherData(vector<byte> *data, int addr, Filesystem fs);
    int getType() const;
    /*********** SETTER ***********/
    void setName(string newName);//set name for a new file/directory    

    /*****************************/
    //mengembalikan alamat blok inode file tersebut, -1 jika disk penuh
    static int mkdir(string name, Filesystem fs, int iparentaddr);
    static File open(int iAddr);
    
protected:
    int size;       //file size
    string name;    //file name    
    int type;     //type, whether it is a file or a directory
    vector<byte> data;
    int iParent; // alamat inode parent
private:
    
};

#endif	/* FILE_H */