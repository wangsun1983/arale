#ifndef __FS_FILE_H__
#define __FS_FILE_H__
#include "fs_utils.h"
#include "fs.h"

int file_create(const char *pathname,partition_data **partition);
int file_rename(const char *pathname,char *newname,partition_data **partition);
int file_remove(const char *pathname,partition_data **partition);

int file_write_overlap(int inode_no,partition_data *partition,char *buff,uint32_t size);
uint32_t file_read(uint32_t inode_no,partition_data *patition,char *buff,int buff_size,int where_to_read);

#endif
