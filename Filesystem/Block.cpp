#include "Block.h"
#include <fstream>

Block::Block(int number,byte *data):number(number) {        
    for(int i=0;i<BLOCK_SIZE;i++) {
        this->data[i] = data[i];         
    }    
}

Block::Block(const Block& orig):number(orig.number) {             
    for(int i=0;i<BLOCK_SIZE;i++) {
        data[i] = orig.data[i];       
    }
}

Block::~Block() {    
}

byte* Block::readBlock() {    
    return readBlock(0,BLOCK_SIZE);    
}

byte* Block::readBlock(int offset, int count) {
    byte *ret = new byte[count];
    
    for(int i=0;i<count;i++) {
        ret[i] = data[offset+i];                    
    }
    return ret;
}

void Block::writeBlock(const char* src, int count) {    
    writeBlock(src,count,0);
}

void Block::writeBlock(const char* src, int count, int idx) {
    for(int i=0;i<count;i++) {
        data[i+idx] = src[i];
    }
}

byte Block::getByte(int pos) {
    return data[pos];
}
