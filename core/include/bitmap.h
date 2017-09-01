#ifndef _BIT_MAP_H_
#define _BIT_MAP_H_

void *create_bitmap(int bits);
void set_bit(char *bitmap,int bits,int on_off);
void set_bit_range(char *bitmap,int on_off,int from,int end);
int get_bit(char *bitmap,int bits);
int count_bit(char *bitmap,int bits,int on_off);
int scan_bit_condition(char *bitmap,int on_off,int size);

#endif
