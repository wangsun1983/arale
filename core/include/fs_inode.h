#ifndef __FS_INODE_H__
#define __FS_INODE_H__

#include "ctype.h"
#include "list.h"

#define MAX_DIR_NUM 512
#define MAX_FILE_NAME_LEN 64

enum INODE_INIT_STATUS
{
     INODE_IDLE = 0,
     INODE_INITED
};

//every file contains 4K
typedef struct file_struct {
    uint32_t inode_no;
    char name[MAX_FILE_NAME_LEN + 1];        //only file's first block contains file name;
    uint32_t start_lba;                      //file start lba
    uint32_t offset;                         //offset is mark last offset of file 
    int type;                                //file may be directory/file
} file_struct;

typedef struct inode {
   uint32_t inode_no;
   uint32_t parent_no;          //we use this no to create directory tree;
   uint32_t inode_open_cnts;
   uint32_t init_status;        //this is use for judge whether child_list has been inited
   file_struct file;            //this file may be directory or file;

   //when fs_init,we will generate the list;
   struct list_head child_list;
   struct list_head parent_ll;
   int status;
}inode;

#endif
