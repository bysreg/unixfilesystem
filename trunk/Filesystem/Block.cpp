#include "Block.h"
#include "Filesystem.h"
#include <fstream>

Block::Block(int number,const byte *data):number(number) {        
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

byte* Block::getBytes(int offset, int count) {
    byte *ret = new byte[count];
    
    for(int i=0;i<count;i++) {
        ret[i] = data[offset+i];                    
    }
    return ret;
}

byte Block::getByte(int pos) const {
    return data[pos];
}

