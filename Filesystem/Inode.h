#ifndef INODE_H
#define	INODE_H

class Inode {
public:
    static const int MAX_ADDRESS_COUNT = 1022;    
    
	int getType() {
		return type;
	}
	
    Inode(int blockaddress, Filesystem filesystem);
    Inode(const Inode& orig);
    virtual ~Inode();
    
private:
	//file attributes    
    int type;
	static const int FILE=0;
	static const int DIR=1;
	
	//alamat block yang memuat data file ini
    int dataaddress[];
    
    //alamat block yang memuat address block
    int otheraddressblock;
 
};

#endif	/* INODE_H */

