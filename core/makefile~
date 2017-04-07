# Entry point of Orange'S
# It must have the same value with 'KernelEntryPointPhyAddr' in load.inc!
ENTRYPOINT	= 0x100000

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

# Programs, flags, etc.
ASM		= nasm
DASM		= ndisasm
CC		= gcc 
LD		= ld
ASMBFLAGS	= -I include/ 
ASMKFLAGS	= -I include/ 
CFLAGS		= -m32 -I include/ -I arch/x86/ -c  -fno-stack-protector
#LDFLAGS		= -m elf_i386 -I include/ -s -Ttext $(ENTRYPOINT)
#LDFLAGS		= -m elf_i386 -I include/ -s -Ttext $(ENTRYPOINT)
LDFLAGS		= -m elf_i386 -I include/ -T linker.ld
#DASMFLAGS	= -u -o $(ENTRYPOINT) -e $(ENTRYOFFSET)
#LDFLAGS		= -m elf_i386 -I include/ 

CORESYS	= ../out/core/CORESE.SYS

LINKO = start \
	core \
        lib_display \
        lib_font \
        lib_pmm \
        lib_vmm \
        lib_error \
        lib_io \
        lib_klibc \



OBJS	= $(OUT_DIR)core.o \
	$(OUT_DIR)start.o \
	$(OUT_DIR)lib_graphic.o \
	$(OUT_DIR)lib_font.o  \
	$(OUT_DIR)lib_pmm.o  \
	$(OUT_DIR)lib_vmm.o  \
	$(OUT_DIR)lib_error.o  \
        $(OUT_DIR)lib_io.o \
        $(OUT_DIR)lib_klibc.o 


everything : $(CORESYS)

start :start.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)start.o start.c

core :core.asm
	$(ASM) -f elf $(ASMBFLAGS) -o $(OUT_DIR)core.o core.asm

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

lib_io :lib/libc/io.asm
	$(ASM) $(ASMBFLAGS) -f elf -o $(OUT_DIR)lib_io.o $(CORE_LIB_C)io.asm

$(CORESYS) : $(LINKO)
	$(LD) $(LDFLAGS) -o ../out/core/CORESE.SYS $(OBJS)



