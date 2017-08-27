#ifndef __FS_FILE_H__
#define __FS_FILE_H__
#include "fs_utils.h"
#include "fs.h"

int file_create(const char *pathname,partition_data **partition);
int file_rename(const char *pathname,char *newname,partition_data **partition);
int file_write_overlap(int inode_no,partition_data *partition,char *buff,uint32_t size);

#endif
