/* 
 * File:   File.h
 * Author: TELKOM-CIS
 *
 * Created on 02 Desember 2011, 21:30
 */

#ifndef FILE_H
#define	FILE_H

#ifndef FILESYSTEM_H
#include "Filesystem.h"
#endif

class File {
public:
    File();
    File(string newName, string absPath, bool type);    //type, true
    File(const File& orig);
    virtual ~File();

    /*********** GETTER ***********/
    int getSize();              //get size for a file
    string getName();           //get local name for a file
    string getAbsoluteName();   //get absoulute name from root

    /*********** SETTER ***********/
    void setName(string newName);//set name for a new file/directory
    void setDir(string absPath); //set directory for this file
private:
    int size;       //file size
    string name;    //file name
    string path;    //path to file
    bool isDir;     //type, whether it is a file or a directory
};

#endif	/* FILE_H */

