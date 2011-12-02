#ifndef INODE_H
#define	INODE_H

class Inode {
public:
    static const int MAX_ADDRESS_COUNT = 8; 
    //file attributes
    //0 = file
    //1 = directory
    int type;
    
    //alamat block yang memuat data file ini
    int dataaddress[];
    
    //alamat block yang memuat address block
    int otheraddressblock;
    
    Inode();
    Inode(const Inode& orig);
    virtual ~Inode();
    
private:

};

#endif	/* INODE_H */

