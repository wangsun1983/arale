#ifndef __FILE_H__
#define __FILE_H__

#include "ctype.h"

#define MAX_DIR_NUM 512
#define MAX_FILE_NAME_LEN 64

//every file contains 4K
typedef struct file_struct {
    uint32_t inode_no;
    char name[MAX_FILE_NAME_LEN];            //only file's first block contains file name;
    uint32_t start_lba;                      //file start lba
    uint32_t start_offset;                   //file's start offset of sector
    uint32_t next_lba;                       //if file is larger than 4k ,we use next lba to mark another file lba
    int type;                                //file may be directory/file
} file_struct;

#endif
