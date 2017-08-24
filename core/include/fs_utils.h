#ifndef __FS_UTILS_H__
#define __FS_UTILS_H__

#include "fs.h"
#include "fs_inode.h"

void split_path(const char *path,char *array[]);
char * file_path_match(const char *pathname,partition_data **patition,inode **inode);

#endif
