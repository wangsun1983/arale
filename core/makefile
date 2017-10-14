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
        
OBJS  = $(OUT_DIR)start.o \
        
everything : $(CORESYS)

include guard/makefile
include task/makefile
include core/makefile
include drivers/makefile
include fs/makefile
include mm/makefile
include lib/makefile
include arch/makefile
include graphic/makefile
#hahah
include ../userspace/makefile

start :start.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)start.o start.c

$(CORESYS) : $(LINKO)
	$(LD) $(LDFLAGS) -o ../out/core/CORESE.SYS $(OBJS)
