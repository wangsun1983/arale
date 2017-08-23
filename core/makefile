# Entry point of Orange'S
# It must have the same value with 'KernelEntryPointPhyAddr' in load.inc!
#ENTRYPOINT	= 0x100000
#ENTRYPOINT	= 0x200000

# Offset of entry point in kernel file
# It depends on ENTRYPOINT
ENTRYOFFSET	=   0x400

#OUT DIR
OUT_DIR 	= ../out/core/

#CORE LIB DIR
CORE_LIB_DIR 	= lib/

#CORE DISPLAY
CORE_LIB_GRAPHIC = graphic/
CORE_LIB_MM = mm/
CORE_LIB_C = lib/libc/
CORE_LIB_ARCH_X86 = arch/x86/
CORE_LIB_DRIVERS = drivers/
CORE_LIB_CORE = core/
CORE_LIB_TASK = task/
CORE_LIB_FS = fs/

#X86 include
CORE_X86_INC = arch/x86/include/

#driver include

# Programs, flags, etc.
ASM		= nasm
DASM		= ndisasm
CC		= gcc
LD		= ld
ASMBFLAGS	= -I include/
ASMKFLAGS	= -I include/
CFLAGS		= -m32 -I include/ -I arch/x86/include -I drivers/include -c -fno-stack-protector
#LDFLAGS		= -m elf_i386 -I include/ -s -Ttext $(ENTRYPOINT)
LDFLAGS		= -m elf_i386 -I include/ -s -Ttext $(ENTRYPOINT)
LDFLAGS		= -m elf_i386 -I include/ -T linker.ld
#DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)
#LDFLAGS		= -m elf_i386 -I include/

CORESYS	= ../out/core/CORESE.SYS

LINKO = start \
        lib_display \
        lib_font \
        lib_pmm \
        lib_vmm \
        lib_mm \
        lib_error \
        lib_io \
        lib_klibc \
        lib_cmos \
        lib_cpu \
        lib_dma \
        lib_except \
        lib_i8253 \
        lib_i8259 \
        lib_idt \
        lib_irq \
        lib_keyboard \
        lib_sysclock \
        lib_gdt \
        lib_task \
        lib_switch \
        lib_bitmap \
        lib_mmzone \
        lib_coalition_allocator \
        lib_fragment_allocator \
        lib_vm_allocator \
        lib_rbtree \
        lib_cache_allocator \
        lib_hdd \
        lib_time \
        lib_sem \
        lib_fs \
				lib_fs_utils \
				lib_fs_dir



OBJS	= $(OUT_DIR)start.o \
        $(OUT_DIR)lib_graphic.o \
        $(OUT_DIR)lib_font.o  \
        $(OUT_DIR)lib_pmm.o  \
        $(OUT_DIR)lib_vmm.o  \
        $(OUT_DIR)lib_error.o  \
        $(OUT_DIR)lib_io.o \
        $(OUT_DIR)lib_klibc.o \
        $(OUT_DIR)lib_cmos.o \
        $(OUT_DIR)lib_cpu.o \
        $(OUT_DIR)lib_dma.o \
        $(OUT_DIR)lib_except.o \
        $(OUT_DIR)lib_idt.o \
        $(OUT_DIR)lib_irq.o \
        $(OUT_DIR)lib_i8253.o \
        $(OUT_DIR)lib_i8259.o \
        $(OUT_DIR)lib_keyboard.o \
        $(OUT_DIR)lib_sysclock.o \
        $(OUT_DIR)lib_gdt.o \
        $(OUT_DIR)lib_mm.o \
        $(OUT_DIR)lib_task.o \
        $(OUT_DIR)lib_switch.o \
        $(OUT_DIR)lib_bitmap.o \
        $(OUT_DIR)lib_mmzone.o \
        $(OUT_DIR)lib_coalition_allocator.o \
        $(OUT_DIR)lib_fragment_allocator.o \
        $(OUT_DIR)lib_vm_allocator.o \
        $(OUT_DIR)lib_rbtree.o \
        $(OUT_DIR)lib_cache_allocator.o \
        $(OUT_DIR)lib_hdd.o \
        $(OUT_DIR)lib_time.o \
        $(OUT_DIR)lib_sem.o \
        $(OUT_DIR)lib_fs.o \
				$(OUT_DIR)lib_fs_utils.o \
				$(OUT_DIR)lib_fs_dir.o

everything : $(CORESYS)

start :start.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)start.o start.c

lib_display :graphic/graphic.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_graphic.o $(CORE_LIB_GRAPHIC)graphic.c

lib_font :graphic/font.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_font.o $(CORE_LIB_GRAPHIC)font.c

lib_pmm :mm/pmm.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_pmm.o $(CORE_LIB_MM)pmm.c

lib_vmm :mm/vmm.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_vmm.o $(CORE_LIB_MM)vmm.c

lib_error :lib/libc/kerror.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_error.o $(CORE_LIB_C)kerror.c

lib_klibc :lib/libc/klibc.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_klibc.o $(CORE_LIB_C)klibc.c

lib_rbtree :lib/libc/rbtree.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_rbtree.o $(CORE_LIB_C)rbtree.c

lib_bitmap :lib/libc/bitmap.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_bitmap.o $(CORE_LIB_C)bitmap.c

lib_io :lib/libc/io.asm
	$(ASM) $(ASMBFLAGS) -f elf -o $(OUT_DIR)lib_io.o $(CORE_LIB_C)io.asm

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

lib_keyboard :drivers/keyboard/keyboard.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_keyboard.o $(CORE_LIB_DRIVERS)keyboard/keyboard.c

lib_hdd :drivers/hdd/hdd.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_hdd.o $(CORE_LIB_DRIVERS)hdd/hdd.c

lib_sysclock :core/sysclock.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sysclock.o $(CORE_LIB_CORE)sysclock.c

lib_time :core/time.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_time.o $(CORE_LIB_CORE)time.c

lib_sem :task/semaphore.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sem.o $(CORE_LIB_TASK)semaphore.c

lib_fs :fs/fs.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_fs.o $(CORE_LIB_FS)fs.c

lib_fs_utils :fs/fs_utils.c
		$(CC) $(CFLAGS) -o $(OUT_DIR)lib_fs_utils.o $(CORE_LIB_FS)fs_utils.c

lib_fs_dir :fs/fs_dir.c
				$(CC) $(CFLAGS) -o $(OUT_DIR)lib_fs_dir.o $(CORE_LIB_FS)fs_dir.c

lib_gdt :mm/gdt.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_gdt.o $(CORE_LIB_MM)gdt.c

lib_mm :mm/mm.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_mm.o $(CORE_LIB_MM)mm.c

lib_fragment_allocator :mm/fragment_allocator.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_fragment_allocator.o $(CORE_LIB_MM)fragment_allocator.c

lib_mmzone :mm/mmzone.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_mmzone.o $(CORE_LIB_MM)mmzone.c

lib_coalition_allocator :mm/coalition_allocator.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_coalition_allocator.o $(CORE_LIB_MM)coalition_allocator.c

lib_vm_allocator :mm/vm_allocator.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_vm_allocator.o $(CORE_LIB_MM)vm_allocator.c

lib_cache_allocator :mm/cache_allocator.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_cache_allocator.o $(CORE_LIB_MM)cache_allocator.c

lib_task :task/task.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_task.o $(CORE_LIB_TASK)task.c

lib_switch :task/switch.asm
	$(ASM) $(ASMBFLAGS) -f elf -o $(OUT_DIR)lib_switch.o $(CORE_LIB_TASK)switch.asm

lib_irq :arch/x86/irq.asm
	$(ASM) $(ASMBFLAGS) -f elf -o $(OUT_DIR)lib_irq.o $(CORE_LIB_ARCH_X86)irq.asm

$(CORESYS) : $(LINKO)
	$(LD) $(LDFLAGS) -o ../out/core/CORESE.SYS $(OBJS)
