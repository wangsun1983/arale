
CORE_LIB_ARCH_X86 = arch/x86/

LINKO += lib_cmos \
         lib_cpu \
         lib_dma \
         lib_except \
         lib_i8253 \
         lib_i8259 \
         lib_idt \
         lib_irq \

OBJS += $(OUT_DIR)lib_cmos.o \
        $(OUT_DIR)lib_cpu.o \
        $(OUT_DIR)lib_dma.o \
        $(OUT_DIR)lib_except.o \
        $(OUT_DIR)lib_idt.o \
        $(OUT_DIR)lib_irq.o \
        $(OUT_DIR)lib_i8253.o \
        $(OUT_DIR)lib_i8259.o \

lib_cmos :arch/x86/cmos.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_cmos.o $(CORE_LIB_ARCH_X86)cmos.c

lib_cpu :arch/x86/cpu.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_cpu.o $(CORE_LIB_ARCH_X86)cpu.c

lib_dma :arch/x86/dma.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_dma.o $(CORE_LIB_ARCH_X86)dma.c

lib_except :arch/x86/except.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_except.o $(CORE_LIB_ARCH_X86)except.c

lib_i8253 :arch/x86/i8253.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_i8253.o $(CORE_LIB_ARCH_X86)i8253.c

lib_i8259 :arch/x86/i8259.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_i8259.o $(CORE_LIB_ARCH_X86)i8259.c

lib_idt :arch/x86/idt.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_idt.o $(CORE_LIB_ARCH_X86)idt.c

lib_irq :arch/x86/irq.asm
	$(ASM) $(ASMBFLAGS) -f elf -o $(OUT_DIR)lib_irq.o $(CORE_LIB_ARCH_X86)irq.asm






