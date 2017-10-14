/******************************************************************************
 *      x86 CPU related stuff.
 *
 ******************************************************************************/

#ifndef CPU_N1YAILHP
#define CPU_N1YAILHP

#include "list.h"
#include "idt.h"

struct x86_reg_t {
    int eax;
    int ebx;
    int ecx;
    int edx;
    int esi;
    int edi;
    int ebp;
    int esp;
    int eip;
    struct x86_seg_reg_t *seg_reg;
};

struct x86_seg_reg_t {
    short cs;
    short ds;
    short ss;
    short es;
    short fs;
    short gs;
};

/* IRQ exception lines */
#define X86_DIVIDE_IRQ  0
#define X86_SINGLE_STEP_DEBUG_IRQ 1
#define X86_NONMASK_IRQ 2
#define X86_BREAKPOINT_IRQ 3
#define X86_OVERFLOW_IRQ 4
#define X86_BOUND_IRQ 5
#define X86_INVALID_OPCODE_IRQ 6
#define X86_BUSY_COPROC_IRQ 7
#define X86_DOUBLE_FAULT_IRQ 8
#define X86_COPROC_OVERRUN_IRQ 9
#define X86_INVALID_TSS_IRQ 10
#define X86_NO_SEGMENT_IRQ 11
#define X86_STACK_IRQ 12
#define X86_GPF_IRQ 13
#define X86_PAGE_FAULT_IRQ 14
#define X86_COPROC_IRQ 16

//=========================== owner interrupt ===============/
#define X86_RE_SCHEDULE 48
#define X86_SYS_CALL 49

#define X86_IRQ_MAX 256

int x86_init(); void x86_cpu_halt();

#ifdef DUMP_CPU_REG
inline int x86_dump_registers();
#endif

unsigned char inportb (unsigned short _port);
void outportb (unsigned short _port, unsigned char _data);
void insw(uint16_t port, void* addr, uint32_t word_cnt);
void outsw(uint16_t port, const void* addr, uint32_t word_cnt);
uint8_t inb(uint16_t port);
uint32_t xchg(volatile uint32_t *addr, uint32_t newval);
/* clear interrupt */
void cli();

/* set interrupt */
void sti();

/* enter halt state, until int comeup */
void hlt();

typedef struct struct_irq_handler
{
    struct list_head ll;
    irq_handler irq_handle;
}struct_irq_handler;

struct list_head irq_list[X86_IRQ_MAX];


#endif /* end of include guard: CPU_N1YAILHP */
