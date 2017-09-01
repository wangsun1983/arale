#include "fs.h"
#include "test_fs.h"
#include "mm.h"

//test file
#define TEST_FILE_PATH  "root0/abc7.txt"

int test_fs_creat()
{
    //get node bit num

    uint32_t fd0 = fs_create(TEST_FILE_PATH,FT_FILE);
    if(fd0 == -1)
    {
        fs_remove(TEST_FILE_PATH,FT_FILE);
        fd0 = fs_create(TEST_FILE_PATH,FT_FILE);
        if(fd0 == -1)
        {
             kprintf("test_fs_creat fail trace1 \n");
             return -1;
        }
    }

    return  1;
}

int test_fs_remove()
{
  //2.fs_read
  char *file = (char *)kmalloc(1024 *8);
  kmemset(file,0,1024*8);
  int index = 0;

  for(;index <1024*8 - 1;index ++)
  {
      file[index] = 'c';
      /*file[index] = (index+1)%255;

      if(file[index] == 0)
      {
           file[index] = 2;
      }
      */
  }

  //uint32_t fd0 = fs_create("root0/abc7.txt",FT_FILE);
  uint32_t fd = fs_open("root0/abc7.txt");
  //kprintf("file length is %d \n",kstrlen(file));
  fs_write(fd,file,kstrlen(file),WRITE_NORMAL);

  char *file1 = "hello";
  fs_write(fd,file1,kstrlen(file1),WRITE_NORMAL);

  char *append = "end3";
  kprintf("file write append start \n");
  fs_write(fd,append,kstrlen(append),WRITE_APPEND);
  kprintf("file write append end \n");

  fs_remove("root0/abc7.txt",FT_FILE);

#if 0
  char *read = (char *)kmalloc(1024 *8);
  kmemset(read,0,1024*8);
  fs_read(fd,read,1024*8,0);
  int check = 0;
  for(;check < 1024*8;check++)
  {
      if(file[check] != read[check])
      {
          kprintf("read error,check is %d,file[check] is %d,read[check] is %d \n"
                  ,check,file[check],read[check]);
          break;
      }
  }
#endif
}

void start_test_fs()
{
    if(test_fs_creat() == -1)
    {
        return;
    }

    //TODO

}
