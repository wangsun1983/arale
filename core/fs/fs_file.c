#include "fs_file.h"
#include "fs.h"

int file_create(const char *pathname,partition_data **partition)
{
    partition_data *find_part;
    inode *parent;
    char *create_file = file_path_match(pathname,&find_part,&parent);

    //TODO
    return  -1;
}
