#include "fs_utils.h"
#include "fs_inode.h"
#include "fs.h"
#include "mm.h"
#include "list.h"

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

int get_path_depth(const char *path)
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

//up_node is parent node
char * file_path_match(const char *pathname,partition_data **patition,inode **up_node)
{
    int length = get_path_depth(pathname);
    int find_index = 0;
    int freecount = 0;
    //char **path = (char **)kmalloc(length);
    char *path[length];
    partition_data *find_part;
    int inode_no = -1;
    split_path(pathname,path);

    char *root = path[0];
    //inode *dir = (inode *)kmalloc(sizeof(inode));

    //we should find the dir's root
    struct list_head *p;
    //kprintf("dir_create trace1 pathname is %s,length is %d \n",pathname,length);
    list_for_each(p,&partition_list) {
        find_part = list_entry(p,partition_data,ll);
        inode *inode_table = find_part->inode_table;
        inode *parent = &inode_table[0];

        if(inode_table != NULL
           && kstrcmp(inode_table[0].file.name,root) == 0)
        {
            //kprintf("find hit,inode_table[0] is %s,root is %s \n",inode_table[0].file.name,root);
            find_index++;
            //last one is the directory which need creat
            while(find_index < length - 1)
            {
                bool is_find = false;
                struct list_head *node;
                list_for_each(node,&parent->child_list) {
                    inode *find_node = list_entry(p,inode,parent_ll);
                    if(kstrcmp(find_node->file.name,path[find_index]) == 0)
                    {
                        parent = find_node;
                        *up_node = parent;
                        find_index ++;
                        is_find = true;
                        break;
                    }
                }

                if(!is_find)
                {
                    //kprintf("dir_create,dir not found \n");
                    find_part = NULL;
                    goto end;
                }
            }

            //check whether the directory already exist.
            //kprintf("parent file name is %s \n",parent->file.name);
            struct list_head *dir_node;
            list_for_each(dir_node,&parent->child_list){
                inode *find_node = list_entry(dir_node,inode,parent_ll);
                //kprintf("find_node->file.name is %s,path is %s \n",find_node->file.name,path[length - 1]);
                if(kstrcmp(find_node->file.name,path[length - 1]) == 0)
                {
                    //kprintf("same folder \n");
                    find_part = NULL;
                    goto end;
                }
            }
        }
    }
end:

    for(;freecount < length - 1;freecount++)
    {
        char *freedata = path[freecount];
        free(freedata);
    }
    if(find_part == NULL)
    {
        return NULL;
    }

    *patition = find_part;
    return path[length-1];
}
