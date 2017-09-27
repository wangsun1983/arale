
CORE_LIB_CORE_CLOCK = core/clock/
CORE_LIB_CORE_COMMON = core/common/
CORE_LIB_CORE_INPUT = core/input/

LINKO += lib_sysclock \
         lib_sys_observer \
         lib_time \
				 lib_keydispatcher \

OBJS +=  $(OUT_DIR)lib_sysclock.o \
         $(OUT_DIR)lib_sys_observer.o \
         $(OUT_DIR)lib_time.o \
				 $(OUT_DIR)lib_keydispatcher.o

lib_sysclock :core/clock/sysclock.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sysclock.o $(CORE_LIB_CORE_CLOCK)sysclock.c

lib_sys_observer :core/common/sys_observer.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sys_observer.o $(CORE_LIB_CORE_COMMON)sys_observer.c

lib_time :core/clock/time.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_time.o $(CORE_LIB_CORE_CLOCK)time.c

lib_keydispatcher :core/input/key_dispatcher.c
		$(CC) $(CFLAGS) -o $(OUT_DIR)lib_keydispatcher.o $(CORE_LIB_CORE_INPUT)key_dispatcher.c
