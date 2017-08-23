#ifndef __INODE_H__
#define __INODE_H__

#include "ctype.h"
#include "list.h"
#include "fs.h"
#include "file.h"

enum INODE_INIT_STATUS
{
     INODE_IDLE = 0,
     INODE_INITED
};

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
