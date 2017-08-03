/******************************************************************************
 *  Intel 8259A PIC - Programmable Interrupt Controller
 *
 *****************************************************************************/

#ifndef I8259_5UIR4IFN
#define I8259_5UIR4IFN



/* IRQ mappings */  /* in 8086 must be 4 bits aligned */
#define IRQ0_8253_VECTOR     (0x20 & ~7) /* 0x0-0x1F is reserved by Intel */
#define IRQ0_VECTOR IRQ0_8253_VECTOR               //IRQ0:Programmable Interrupt Timer Interrupt

#define IRQ1_KBR_VECTOR     (IRQ0_VECTOR + 1)      //IRQ1:Keyboard Interrypt
#define IRQ2_IDE0_VECTOR     (IRQ0_VECTOR + 2)     //IRQ2:Cacade(used internally by the two PICS )
#define IRQ3_IDE1_VECTOR     (IRQ0_VECTOR + 3)     //IRQ3:COM2(if enabled)
#define IRQ4_VECTOR     (IRQ0_VECTOR + 4)          //IRQ4:COM1(if enabled)
#define IRQ5_VECTOR     (IRQ0_VECTOR + 5)          //IRQ5:LPT2(if enabled)
#define IRQ6_VECTOR     (IRQ0_VECTOR + 6)          //IRQ6:Floppy Disk
#define IRQ7_VECTOR     (IRQ0_VECTOR + 7)          //IRQ7:LPT1
#define IRQ8_VECTOR     (IRQ0_VECTOR + 8)          //IRQ8:CMOS real-time clock
#define IRQ9_VECTOR     (IRQ0_VECTOR + 9)          //IRQ9:Free for peripherals/legacy scsi/NIC
#define IRQ10_VECTOR    (IRQ0_VECTOR + 10)         //IRQ10:Free for peripherals/SCSI/NIC
#define IRQ11_VECTOR    (IRQ0_VECTOR + 11)         //IRQ11:Free for peripherals/SCSI/NIC
#define IRQ12_VECTOR    (IRQ0_VECTOR + 12)         //IRQ12:PS2 Mouse
#define IRQ13_VECTOR    (IRQ0_VECTOR + 13)         //IRQ13:FPU/Coprocessor/Inter-processor
#define IRQ14_ATA_P_VECTOR    (IRQ0_VECTOR + 14)     //IRQ14:Primary ATA Hard Disk
#define IRQ15_ATA_S_VECTOR    (IRQ0_VECTOR + 15)         //IRQ15:Secondary ATA Hard Disk

#define IRQ15_VECTOR IRQ15_ATA_S_VECTOR

#define IS_PIC1_LINE(irq_line)  \
        ((irq_line >= IRQ0_VECTOR) && (irq_line <= IRQ7_VECTOR))
#define IS_PIC2_LINE(irq_line)  \
        ((irq_line >= IRQ8_VECTOR) && (irq_line <= IRQ15_VECTOR))
#define IS_PIC_LINE(irq_line)   \
        (IS_PIC1_LINE(irq_line) || IS_PIC2_LINE(irq_line))

int i8259_init();
int irq_done(int irq);
inline int irq_disable();
inline int irq_enable();

#endif /* end of include guard: I8259_5UIR4IFN */
