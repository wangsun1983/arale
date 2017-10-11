#include "mouse.h"
#include "log.h"
#include "cpu.h"

/* Ports */
#define KBRD_PORT_ENCODER 0x60 /* controller inside the keyboard itself */
#define KBRD_PORT_CTRL 0x64  /* i8042 controller on the moherboard */

private void mouse_enable();

public void x86_mouse_irq_do_handle()
{
    uint8_t data;
    LOGD("======wokao======= \n");
    return;
}

public void mouse_init()
{
    LOGD("mouse init \n");
    mouse_enable();
}

private void mouse_enable()
{
    //outportb(0x64,0xd4);
    //outportb(0x60,0xf4);
}
