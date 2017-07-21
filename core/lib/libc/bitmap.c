#include "bitmap.h"
#include "mm.h"
#include "klibc.h"

#define BIT_0 0
#define BIT_1 1

#define BYTE_PER_BITS 8
#define BIT_MASK 0x01

void *create_bitmap(int bits) {
    char *map = (char*)vmalloc(bits/BYTE_PER_BITS + 1);
    kmemset(map,0,bits/BYTE_PER_BITS + 1);
    return map;
}

void set_bit(char *bitmap,int bits,int on_off) {
    
    int block = bits/BYTE_PER_BITS;
    int offset = bits%BYTE_PER_BITS;

    char value = bitmap[block];
    
    if(on_off) {
        value |= BIT_1 << offset;
    } else {
        value &= BIT_0 << offset;
    }

    bitmap[block] = value;
}

void set_bit_range(char *bitmap,int on_off,int from,int end) {

    int start_block = from/BYTE_PER_BITS;
    int start_offset = from%BYTE_PER_BITS;

    int end_block = end/BYTE_PER_BITS;
    int end_offset = end%BYTE_PER_BITS;
    
    for(;start_block <= end_block;start_block++) {
        
        int start_bit = 0;
        int end_bit = 0;

        if(start_block == from/BYTE_PER_BITS) {
            //if it is the first block
            if(end_block == start_block) {
                start_bit = start_offset;
                end_bit = end_offset;
            } else {
                start_bit = start_offset;
                end_bit = BYTE_PER_BITS;
            }
        } else if(start_block == end_block) {
            //if it is the last block
            start_bit = 0;
            end_bit = end_offset;
        } else {
            //if it is the middle block
            bitmap[start_block] = 0xff;
            continue;
        }

        for(;start_bit <= end_bit;start_bit++) {
            if(on_off) {
                bitmap[start_block] |= BIT_1 << start_bit;
            } else {
                bitmap[start_block] &= BIT_0 << start_bit;
            }
        }
    }
}



int get_bit(char *bitmap,int bits) {
    
    int block = bits/BYTE_PER_BITS;
    int offset = bits%BYTE_PER_BITS;

    char value = bitmap[block];
    return (value>>offset)&BIT_MASK; 
}

#if 0
int main() {

    char *map = create_bitmap(1024);

    set_bit(map,0,1);
    set_bit(map,1,1);
    set_bit(map,2,1);
    set_bit(map,3,1);
    set_bit(map,4,1);
    set_bit(map,5,1);
    set_bit(map,6,1);
    set_bit(map,7,1);
    printf("1.getmap is %d map is %d \n",get_bit(map,1),*map);


    //set_bit_range(map,1,0,3);
    //printf("2.getmap is %d map is %d \n",get_bit(map,1),*map);

    set_bit_range(map,1,76,1023);
    int i = 0;
    for(;i < 1024;i++) {
        printf("bit[%d] is %d \n",i,get_bit(map,i));
    }
}
#endif
