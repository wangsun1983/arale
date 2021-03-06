#ifndef __DIR_H__
#define __DIR_H__
#include "ctype.h"
#include "fs.h"

int dir_create(const char *pathname,partition_data **partition);
int dir_remove(const char *pathname,partition_data **partition);
int dir_rename(const char *pathname,char *newname,partition_data **partition);

enum DIR_REMOVE_RESULT
{
    REMOVE_FAIL_NOT_FOUND = -100,
    REMOVE_FAIL_FILE_EXIT,
};

#endif
