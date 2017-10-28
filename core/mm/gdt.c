/**************************************************************
 CopyRight     :No
 FileName      :gdt.c
 Author        :Sunli.Wang
 Version       :0.01
 Date          :20171010
 Description   :gdt config file
***************************************************************/
#include "gdt.h"
#include "log.h"

/*----------------------------------------------
                local data
----------------------------------------------*/
private seg_descriptor gdt[16] =
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

private struct descriptor_addr gdt_pd = {
	sizeof(gdt) - 1, (unsigned long) gdt
};

/*----------------------------------------------
                local method
----------------------------------------------*/
private void lgdt(void *p);

/*----------------------------------------------
                public method
----------------------------------------------*/
public void init_gdt()
{
    lgdt(&gdt_pd);
    addr_t esp;

    asm volatile("movw %%ax,%%gs" :: "a" (_USER_DS_ | 3));
    asm volatile("movw %%ax,%%fs" :: "a" (_USER_DS_ | 3));
    asm volatile("movw %%ax,%%es" :: "a" (_KERNEL_DS_));
    asm volatile("movw %%ax,%%ds" :: "a" (_KERNEL_DS_));
    asm volatile("movw %%ax,%%ss" :: "a" (_KERNEL_DS_));

#ifdef DEBUG_CS
		addr_t val;
		__asm __volatile("movw %%cs,%0" : "=r" (val));
		LOGD("task cs is %x \n",val);
		__asm __volatile("movw %%ds,%0" : "=r" (val));
		LOGD("task ds is %x \n",val);
#endif

//    asm("movl %%esp,%0":"=m"(esp));
//    LOGD("esp is %x \n",esp);

//    asm volatile("movl $0x300000,%%eax \n"
//                 "movl %%eax,%%esp \n"
//                  ::: "eax");
//    LOGD("esp2 is %x \n",esp);
    asm volatile("ljmp %0,$1f\n 1:\n" :: "i" (_KERNEL_CS_));

#ifdef DEBUG_CS
		__asm __volatile("movw %%cs,%0" : "=r" (val));
		LOGD("task2 cs is %x \n",val);
		__asm __volatile("movw %%ds,%0" : "=r" (val));
		LOGD("task2 ds is %x \n",val);
#endif

}

/*----------------------------------------------
                private method
----------------------------------------------*/
private void lgdt(void *p)
{
	__asm __volatile("lgdt (%0)" : : "r" (p));
}
