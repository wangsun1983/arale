/******************************************************************************
 *      x86 CPU exception handlers.
 *
 ******************************************************************************/

#include "cpu.h"
#include "ctype.h"
#include "klibc.h"
#include "task.h"
#include "idt.h"
#include "list.h"
#include "log.h"
#include "core_sys_call.h"

//extern void kernel_panic(char *msg);
#define kernel_panic LOGD


static void local_irq_call(int irq_no)
{
    struct list_head* _irq = &irq_list[irq_no];
    if(!list_empty(_irq))
    {
        struct list_head *p;
        list_for_each(p,_irq) {
            struct_irq_handler *handler = list_entry(p,struct_irq_handler,ll);
            handler->irq_handle();
        }
    }
}

/*
 * Divide by zero exception handler.
 * DIV and IDIV instructions can cause it.
 * IRQ: 0
 */
void x86_divide_except(struct interrupt_frame *frame)
{
    kernel_panic("attempt to divide by zero");
}

/*
 * Occurs during various breakpoint traps and faults.
 * IRQ: 1
 */
void x86_single_step_debug_except(struct interrupt_frame *frame)
{
    kernel_panic("single step debug trap");
}

/*
 * Occurs during nonmaskable hardware interrupt.
 * IRQ: 2
 */
void x86_nonmask_except(struct interrupt_frame *frame)
{
    kernel_panic("nonmaskable hardware interrupt");
}

/*
 * Occurs when CPU encounters INT 3 instruction.
 * IRQ: 3
 */
void x86_breakpoint_except(struct interrupt_frame *frame)
{
    kernel_panic("breakpoint INT 3 instruction");
}

/*
 * Occurs when CPU encounters INT0 instruction while OF flag is set.
 * IRQ: 4
 */
void x86_overflow_except(struct interrupt_frame *frame)
{
    kernel_panic("overflow fault");
}

/*
 * Occurs when BOUND instructions operand exceeds specified limit.
 * IRQ: 5
 */
void x86_bound_except(struct interrupt_frame *frame)
{
    kernel_panic("BOUND instruction fault");
}

/*
 * Invalid opcode exception.
 * IRQ: 6
 */
void x86_invalid_opcode_except(struct interrupt_frame *frame)
{
    kernel_panic("invalid opcode");
}

/*
 * Occurs during one of the two conditions:
 *      - CPU reaches ESC instruction while EM (emulate) bit of CR0 is set.
 *      - CPU reaches WAIT or ESC instruction and both MP (monitor coprocessor)
 *      and TS (task switched) bits of CR0 are set.
 * IRQ: 7
 */
void x86_busy_coproc_except(struct interrupt_frame *frame)
{
    kernel_panic("busy co-CPU fault");
}

/*
 * Occurs when CPU detects an exception while trying to invoke prior
 * exception handler.
 * IRQ: 8
 */
void x86_double_fault_except(struct interrupt_frame *frame)
{
    kernel_panic("double fault");
}

/*
 * Occurs when a page or segment violation is detected while transferring the
 * middle portion of a coprocessor operand to the NPX.
 * IRQ: 9
 * NOTE: 386 or earlier only.
 */
void x86_coproc_overrun_except(struct interrupt_frame *frame)
{
    kernel_panic("co-CPU overrun fault");
}

/*
 * Occurs if during a task switch the new TSS is invalid.
 * IRQ: 10
 */
void x86_invalid_tss_except(struct interrupt_frame *frame)
{
    kernel_panic("invalid TSS");
}

/*
 * Occurs when CPU detects that the present bit of a descriptor is zero.
 * IRQ: 11
 */
void x86_no_segment_except(struct interrupt_frame *frame)
{
    kernel_panic("no segment fault");
}

/*
 * Occurs during one of two conditions:
 *      - Limit violation in any operation that refers to
 *      SS (stack segment register).
 *      - When attempting to load SS with a descriptor which is marked
 *      as not-present but is otherwise valid.
 * IRQ: 12
 */
void x86_stack_except(struct interrupt_frame *frame)
{
    kernel_panic("stack fault");
}

/*
 * Occurs during all the rest of protection violations
 * General Protection Exception
 * IRQ: 13
 */
void x86_gpf_except(struct interrupt_frame *frame)
{
    kernel_panic("GPF");
    x86_cpu_halt();
}

/*
 * Page translation exception.
 * IRQ: 14
 */
void x86_page_fault_except(struct interrupt_frame *frame)
{
    uint32_t val;
    __asm __volatile("movl %%cr2,%0" : "=r" (val));
    LOGD("page fault addr is %x \n",val);

    //we use halt to free system .
    x86_cpu_halt();
    //LOGD("frame ip is %d,cs is %d",frame->eip,frame->cs);
/*
    task_struct *current = GET_CURRENT_TASK();
    LOGD("exception error current mm is %x \n",current->mm);
    LOGD("exception error core mem  is %x \n",&core_mem);
    //we should alloc pem for current page
    addr_t mem = 0;

    mem = pmm_alloc(PAGE_SIZE);

    int _pd = (val >>22) & 0x3FF; //max is 1024=>2^10
    int _pt = (val >>12) & 0x3FF;

    addr_t i  = PD_ENTRY_CNT*(_pd - memory_range_user.start_pgd) + _pt;
    addr_t *ptem = current->mm->pte_user;
    LOGD("pgd before status is %x,pte is %x \n",current->mm->pgd[_pd],ptem[i]);
    //LOGD("exception pd is %d,start pgd is %X ,pt is %d,i is %d mem is %x \n",_pd,memory_range_user.start_pgd,_pt,i,mem);
    ptem[i] = mem | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;

    current->mm->pgd[_pd] = ((current->mm->pgd[_pd] >>12)<<12) | ENTRY_PRESENT | ENTRY_RW | ENTRY_SUPERVISOR;
    LOGD("pgd after status is %x,pte is %x \n",current->mm->pgd[_pd],ptem[i]);
    //pte = (addr_t *)core_mem.pte_user;

    refresh_tlb(current->mm->pgd, val);

    //load_pd(current->mm->pgd);
    //kernel_panic("page fault ");
    //while(1){}
*/
}

/* IRQ 15 is reserved */

/*
 * Occurs when CPU detects a signal from the coCPU on the ERROR# input pin.
 * IRQ: 16
 */
void x86_coproc_except(struct interrupt_frame *frame)
{
    kernel_panic("internal co-CPU fault");
}

/* IRQ 17-31 are reserved */

//wangsl
void x86_resched_do_handler()
{
    //LOGD("x86_resched \n");
    //task_scheduler();
}

void x86_sys_call_do_handler(struct interrupt_frame *frame)
{
    //uint32_t result = core_syscall_handler(frame.eax, frame.edx, frame.ecx,
    //                 frame.ebx, frame.edi, frame.esi);

    uint32_t result = core_syscall_handler(frame->eax, frame->edx, frame->ecx,
    frame->ebx, frame->edi, frame->esi);
    frame->eax = result;
}
