USERSPACE_LIB_LIBC = ../userspace/libc/
USERSPACE_LIB_SYSCALL = ../userspace/libc/syscall/
USERSPACE_LIB_THREAD = ../userspace/libc/thread/

LINKO += lib_common \
         lib_printf \
         lib_string \
         lib_sys_call \
         lib_sys_call_putchar \
				 lib_sys_call_thread \
				 lib_sys_call_ipcthread \
				 lib_pthread \


OBJS += $(OUT_DIR)lib_common.o \
        $(OUT_DIR)lib_printf.o \
        $(OUT_DIR)lib_string.o \
				$(OUT_DIR)lib_pthread.o \
        $(OUT_DIR)lib_sys_call.o \
        $(OUT_DIR)lib_sys_call_putchar.o \
				$(OUT_DIR)lib_sys_call_thread.o \
				$(OUT_DIR)lib_sys_call_ipcthread.o \

lib_common :../userspace/libc/common.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_common.o $(USERSPACE_LIB_LIBC)common.c

lib_printf :../userspace/libc/printf.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_printf.o $(USERSPACE_LIB_LIBC)printf.c

lib_string :../userspace/libc/string.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_string.o $(USERSPACE_LIB_LIBC)string.c

lib_pthread :../userspace/libc/thread/pthread.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_pthread.o $(USERSPACE_LIB_THREAD)pthread.c

lib_sys_call :../userspace/libc/syscall/sys_call.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sys_call.o $(USERSPACE_LIB_SYSCALL)sys_call.c

lib_sys_call_ipcthread :../userspace/libc/syscall/sys_call_ipcthread.c
		$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sys_call_ipcthread.o $(USERSPACE_LIB_SYSCALL)sys_call_ipcthread.c

lib_sys_call_putchar :../userspace/libc/syscall/sys_call_putchar.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sys_call_putchar.o $(USERSPACE_LIB_SYSCALL)sys_call_putchar.c

lib_sys_call_thread :../userspace/libc/syscall/sys_call_thread.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sys_call_thread.o $(USERSPACE_LIB_SYSCALL)sys_call_thread.c
