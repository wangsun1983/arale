//#include <math.h>
#include "color.h"
#include "io.h"
#include "font.h"

#define VRAM  0xa0000

#define DISP_WIDTH 320
#define DISP_HEIGHT 200

static char *vram;

char font_cursor[16][16] = {
        "**************..",
        "*OOOOOOOOOOO*...",
        "*OOOOOOOOOO*....",
        "*OOOOOOOOO*.....",
        "*OOOOOOOO*......",
        "*OOOOOOO*.......",
        "*OOOOOOO*.......",
        "*OOOOOOOO*......",
        "*OOOO**OOO*.....",
        "*OOO*..*OOO*....",
        "*OO*....*OOO*...",
        "*O*......*OOO*..",
        "**........*OOO*.",
        "*..........*OOO*",
        "............*OO*",
        ".............***"
};

char mouse[256];

void set_palette(int start)
{
    int i, eflags;
    eflags = io_load_eflags();
    io_cli();
    io_out8(0x03c8, start);
    io_out8(0x03c9,0x00); io_out8(0x03c9,0x00); io_out8(0x03c9,0x00);   //黑
    io_out8(0x03c9,0xff); io_out8(0x03c9,0x00); io_out8(0x03c9,0x00);   //亮红
    io_out8(0x03c9,0x00); io_out8(0x03c9,0xff); io_out8(0x03c9,0x00);   //亮绿
    io_out8(0x03c9,0xff); io_out8(0x03c9,0xff); io_out8(0x03c9,0x00);   //亮黄
    io_out8(0x03c9,0x00); io_out8(0x03c9,0x00); io_out8(0x03c9,0xff);   //亮蓝
    io_out8(0x03c9,0xff); io_out8(0x03c9,0x00); io_out8(0x03c9,0xff);   //亮紫
    io_out8(0x03c9,0x00); io_out8(0x03c9,0xff); io_out8(0x03c9,0xff);   //浅亮蓝
    io_out8(0x03c9,0xff); io_out8(0x03c9,0xff); io_out8(0x03c9,0xff);   //白
    io_out8(0x03c9,0xc6); io_out8(0x03c9,0xc6); io_out8(0x03c9,0xc6);   //亮灰
    io_out8(0x03c9,0x64); io_out8(0x03c9,0x00); io_out8(0x03c9,0x00);   //暗红
    io_out8(0x03c9,0x00); io_out8(0x03c9,0x64); io_out8(0x03c9,0x00);   //暗绿
    io_out8(0x03c9,0x64); io_out8(0x03c9,0x64); io_out8(0x03c9,0x00);   //暗黄
    io_out8(0x03c9,0x00); io_out8(0x03c9,0x00); io_out8(0x03c9,0x64);   //青
    io_out8(0x03c9,0x64); io_out8(0x03c9,0x00); io_out8(0x03c9,0x64);   //暗紫
    io_out8(0x03c9,0x00); io_out8(0x03c9,0x64); io_out8(0x03c9,0x64);   //浅暗蓝
    io_out8(0x03c9,0x64); io_out8(0x03c9,0x64); io_out8(0x03c9,0x64);   //暗灰
    io_store_eflags(eflags);
}

//we should set the palette
void init_graphic()
{
    set_palette(0);
    vram = (char *)VRAM;

    int x, y;
    for(y = 0;y<16;y++)
    {
        for(x = 0;x<16;x++)
        {
            if (font_cursor[y][x] == '*') {
                mouse[y * 16 + x] = BLACK;
                continue;
            }

            if (font_cursor[y][x] == 'O') {
                mouse[y * 16 + x] = WHITE;
                continue;
            }

            if (font_cursor[y][x] == '.') {
                mouse[y * 16 + x] = -1;
                continue;
            }
        }
    }
}

void disp_font(int line,int startX,char *font,int color)
{
    int i;

    for(i=0;i<16;i++)
    {
        char val = font[i];
        if((val&0xff) == 0xff){continue;}
        if((val&0x80) != 0){vram[DISP_WIDTH*line + startX + 0] = color;}
        if((val&0x40) != 0){vram[DISP_WIDTH*line + startX + 1] = color;}
        if((val&0x20) != 0){vram[DISP_WIDTH*line + startX + 2] = color;}
        if((val&0x10) != 0){vram[DISP_WIDTH*line + startX + 3] = color;}
        if((val&0x08) != 0){vram[DISP_WIDTH*line + startX + 4] = color;}
        if((val&0x04) != 0){vram[DISP_WIDTH*line + startX + 5] = color;}
        if((val&0x02) != 0){vram[DISP_WIDTH*line + startX + 6] = color;}
        if((val&0x01) != 0){vram[DISP_WIDTH*line + startX + 7] = color;}
        line++;
    }
}

void disp_string(char *str,int color)
{
    //int size = strlen(str);
    int index = 0;

    while(str[0] != 0)
    {
        disp_font(75,75+FONT_WIDTH*index,font_list[(char)*str],color);
        str++;
        index++;
    }
}

void refresh()
{
    int i = 0;
    for (i = 0; i <= 0xffff; i++)
    {
         vram[i] = LIGHT_BLUE;
    }

}

void dispWelcome()
{
    char *p = "WELCOME TO ARALE OS";
    disp_string(p,WHITE);
}

void start_refresh()
{
    int i = 0;
    for (i = 0; i <= 0xffff; i++)
    {
         vram[i] = LIGHT_BLUE;
    }

    dispWelcome();

    refresh_mouse(20,20);
}

void refresh_mouse(int x,int y)
{
    int line = 0;
    int axis = 0;

    int shift = y*DISP_WIDTH+x;

    for(line = 0;line<16;line++)
    {
        for(axis = 0;axis<16;axis++)
        {
            if(mouse[line*16+axis] != -1)
            {
                vram[line*DISP_WIDTH + axis + shift] = mouse[line*16+axis];
            }
        }
    }
}
//
