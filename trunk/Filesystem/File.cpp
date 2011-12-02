/* 
 * File:   File.cpp
 * Author: TELKOM-CIS
 * 
 * Created on 02 Desember 2011, 21:30
 */

#include "File.h"

File::File() {

}

File::File(const File& orig) {

}

File::~File() {

}

int File::getSize() {
    return this->size;
}

string File::getName() {
    return this->name;
}

string File::getAbsoluteName() {
    return this->path.append(this->name);
}