#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "global.h"
#include "color.h"
#include "font.h"
#include "mm.h"
#include "vmm.h"

//extern void write_mem8(int addr,int data);
extern void init_font();
extern void init_graphic();
extern void start_refresh();

void start_core()
{
   //init memory first
      

   //init display modules
   init_font();
   init_graphic();
   start_refresh();

   //
}

