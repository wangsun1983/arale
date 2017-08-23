#include "fs_utils.h"

void split_path(const char *path,char *array[])
{
    int length = kstrlen(path);
    int start = 0;
    int cursor = 0;
    int count = 0;

    while(cursor < length)
    {
        if(path[cursor] == '/')
        {
            if(cursor - start != 0)
            {
                 char *p1 = (char *)kmalloc(cursor-start);
                 kmemcpy(p1,&path[start],cursor-start);
                 array[count] = p1;
                 count++;
                 start = cursor++;
            }

            while(path[start] == '/')
            {
                start++;
                cursor = start;
            }
        }

        cursor++;
    }

    if(start < length)
    {
        char *p1 = (char *)kmalloc(length - start + 1);
        kmemcpy(p1,&path[start],length-start);
        array[count] = p1;
    }
}

int get_path_depth(char *path)
{
    int length = kstrlen(path);
    int start = 0;
    int cursor = 0;
    int count = 0;

    while(cursor < length)
    {
        if(path[cursor] == '/')
        {

            if(cursor - start != 0)
            {
                count++;
                start = cursor++;
            }

            while(path[start] == '/')
            {
                start++;
                cursor = start;
            }
        }

        cursor++;
    }

    if(start < length)
    {
        count++;
    }

    return count;
}
