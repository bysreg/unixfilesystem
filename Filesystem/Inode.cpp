#include "Inode.h"

Inode::Inode(int blockaddress, Filesystem filesystem) {
	Block block;
	filesystem.getBlock(blockaddress,block);
	type = ByteUtil::bytesToInt(block.readBlock(0, 4));
	otheraddressblock = ByteUtil::bytesToInt(block.readBlock(4, 4));
	for(int i=8;i<MAX_ADDRESS_COUNT;i++) {
	dataaddress[i-8] = ByteUtil::bytesToInt(block.readBlock(i, 4));
    }
}

Inode::Inode(const Inode& orig) {

}

Inode::~Inode() {
}

