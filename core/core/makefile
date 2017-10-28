
CORE_LIB_CORE_CLOCK = core/clock/
CORE_LIB_CORE_COMMON = core/common/
CORE_LIB_CORE_INPUT = core/input/
CORE_LIB_CORE_SYSCALL = core/syscall/
CORE_LIB_CORE_CONSOLE = core/console/

LINKO += lib_sysclock \
         lib_sys_observer \
         lib_time \
         lib_keydispatcher \
				 lib_syscall \
				 lib_syscall_getpid \
				 lib_syscall_putchar \
				 lib_syscall_thread \
				 lib_console_dispatcher \

OBJS +=  $(OUT_DIR)lib_sysclock.o \
         $(OUT_DIR)lib_sys_observer.o \
         $(OUT_DIR)lib_time.o \
         $(OUT_DIR)lib_keydispatcher.o \
				 $(OUT_DIR)lib_syscall.o \
				 $(OUT_DIR)lib_syscall_getpid.o \
				 $(OUT_DIR)lib_syscall_putchar.o \
				 $(OUT_DIR)lib_syscall_thread.o \
				 $(OUT_DIR)lib_console_dispatcher.o \

lib_sysclock :core/clock/sysclock.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sysclock.o $(CORE_LIB_CORE_CLOCK)sysclock.c

lib_sys_observer :core/common/sys_observer.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sys_observer.o $(CORE_LIB_CORE_COMMON)sys_observer.c

lib_time :core/clock/time.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_time.o $(CORE_LIB_CORE_CLOCK)time.c

lib_keydispatcher :core/input/key_dispatcher.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_keydispatcher.o $(CORE_LIB_CORE_INPUT)key_dispatcher.c

lib_syscall :core/syscall/core_sys_call.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_syscall.o $(CORE_LIB_CORE_SYSCALL)core_sys_call.c

lib_syscall_getpid :core/syscall/core_sys_call_getpid.c
		$(CC) $(CFLAGS) -o $(OUT_DIR)lib_syscall_getpid.o $(CORE_LIB_CORE_SYSCALL)core_sys_call_getpid.c

lib_syscall_thread :core/syscall/core_sys_call_thread.c
		$(CC) $(CFLAGS) -o $(OUT_DIR)lib_syscall_thread.o $(CORE_LIB_CORE_SYSCALL)core_sys_call_thread.c

lib_syscall_putchar :core/syscall/core_sys_call_putchar.c
		$(CC) $(CFLAGS) -o $(OUT_DIR)lib_syscall_putchar.o $(CORE_LIB_CORE_SYSCALL)core_sys_call_putchar.c

lib_console_dispatcher :core/console/console_dispatcher.c
		$(CC) $(CFLAGS) -o $(OUT_DIR)lib_console_dispatcher.o $(CORE_LIB_CORE_CONSOLE)console_dispatcher.c
