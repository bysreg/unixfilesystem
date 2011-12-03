#include <string>
#include "byte.h"
#include "Filesystem.h"

using namespace std;

#ifndef FILEX_H
#define	FILEX_H

class Filex {
public:
    //buka file pada alamat blok yang ditunjuk inode
    Filex(int inodeblock, Filesystem filesystem);
    Filex(const Filex& orig);
    virtual ~Filex();
    
    bool isDirectory();
    bool isFile();
      
    string print();
    
private:
    //nama file atau folder
    string name;
    int type;//0==file 1==direktori    
    byte *data;//data file 
    
    //buka file pada alamat block
    static Filex openFile(int inodeblock);
    static Filex openDir(int inodelock);
    static Filex mkDir(int );    
};

#endif	/* FILEX_H */

