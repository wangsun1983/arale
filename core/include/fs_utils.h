#ifndef __FS_UTILS_H__
#define __FS_UTILS_H__

#include "fs.h"
#include "fs_file.h"
#include "fs_inode.h"

enum MATCH_RESULT
{
    MATCH_SAME_FILE = -100,
    MATCH_PARENT_NOT_FOUND,
    MATCH_FILE_NAME_OVERFLOW,
};

typedef struct fd2inodeData {
    uint8_t patition_no;
    uint32_t inode_no;
}fd2inodeData;

void split_path(const char *path,char *array[]);
addr_t file_path_match(const char *pathname,partition_data **patition,inode **node,inode **select_node);
uint32_t inode2fd(fd2inodeData *data);
void fd2inode(uint32_t fd,fd2inodeData *result);

#endif
