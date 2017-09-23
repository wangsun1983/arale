
CORE_LIB_CORE = core/

LINKO += lib_sysclock \
         lib_sys_observer \
         lib_time

OBJS +=  $(OUT_DIR)lib_sysclock.o \
         $(OUT_DIR)lib_sys_observer.o \
         $(OUT_DIR)lib_time.o 

lib_sysclock :core/sysclock.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sysclock.o $(CORE_LIB_CORE)sysclock.c

lib_sys_observer :core/sys_observer.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sys_observer.o $(CORE_LIB_CORE)sys_observer.c

lib_time :core/time.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_time.o $(CORE_LIB_CORE)time.c



