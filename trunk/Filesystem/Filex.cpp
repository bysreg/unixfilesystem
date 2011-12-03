#include "Filex.h"

Filex::Filex(int numblock, Filesystem fs) {
    
}

Filex::Filex(const Filex& orig) {
}

Filex::~Filex() {
}

bool Filex::isDirectory() {
    return type==1;
}

bool Filex::isFile() {
    return type==0;
}

Filex Filex::openFile(int inodeblock) {
    
}

Filex Filex::openDir(int inodeblock) {
    
}

string Filex::print() {
    
}
