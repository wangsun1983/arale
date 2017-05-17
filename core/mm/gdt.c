#include "gdt.h"

seg_descriptor gdt[16] =
{
	// 0x0 - unused (always faults -- for trapping NULL far pointers)
	[0] = set_seg_null,

	// 0x8 - kernel code segment
	[_KERNEL_CS_ >> 3] = set_seg(STA_X | STA_R, 0x0, 0xffffffff, 0),

	// 0x10 - kernel data segment
	[_KERNEL_DS_ >> 3] = set_seg(STA_W, 0x0, 0xffffffff, 0),

	// 0x18 - user code segment
	[_USER_CS_ >> 3] = set_seg(STA_X | STA_R, 0x0, 0xffffffff, 3),

	// 0x20 - user data segment
	[_USER_DS_ >> 3] = set_seg(STA_W, 0x0, 0xffffffff, 3),

	// Per-CPU TSS descriptors (starting from GD_TSS0) are initialized
	// in trap_init_percpu()
	[_TSS0_ >> 3] = set_seg_null
};

struct descriptor_addr gdt_pd = {
	sizeof(gdt) - 1, (unsigned long) gdt
};

static void lgdt(void *p)
{
	__asm __volatile("lgdt (%0)" : : "r" (p));
}

void init_gdt() 
{
    lgdt(&gdt_pd);
    addr_t esp;

    asm volatile("movw %%ax,%%gs" :: "a" (_USER_DS_ | 3));
    asm volatile("movw %%ax,%%fs" :: "a" (_USER_DS_ | 3));
    asm volatile("movw %%ax,%%es" :: "a" (_KERNEL_DS_));
    asm volatile("movw %%ax,%%ds" :: "a" (_KERNEL_DS_));
    asm volatile("movw %%ax,%%ss" :: "a" (_KERNEL_DS_));
//    asm("movl %%esp,%0":"=m"(esp));
//    printf("esp is %x \n",esp);

//    asm volatile("movl $0x300000,%%eax \n"
//                 "movl %%eax,%%esp \n"
//                  ::: "eax");
//    printf("esp2 is %x \n",esp);
    asm volatile("ljmp %0,$1f\n 1:\n" :: "i" (_KERNEL_CS_));

}
