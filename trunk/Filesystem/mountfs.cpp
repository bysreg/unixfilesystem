/* 
 * File:   mountfs.cpp
 * Author: TELKOM-CIS
 * 
 * Created on 02 Desember 2011, 20:03
 */

#include "mountfs.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

mountfs::mountfs() {

}

mountfs::mountfs(const mountfs& orig) {

}

mountfs::~mountfs() {

}

void mountfs::mount(string path) {
    Filesystem fs(path);
    // ========== debug mode ==========
    cout<< "size = " << fs.getSize();
}

/*
int main(int argc, char** argv) {
    
    string path;
    cout<<  "Enter filesystem path: ";
    cin>>   path;
   
    
    if (argc != 3) {
        cout << "mounting failed, wrong params";
        exit(0);
    }
    path = argv[1];
    string mountpt = argv[2];

    
    mountfs::mount(path);
    
    return 0;
}
*/