
CORE_LIB_TASK = task/

LINKO += lib_task \
         lib_switch \
         lib_dependent_task \
         lib_independent_task \
         lib_sched_deuce \
         lib_mutex \
         lib_sem \

         

OBJS += $(OUT_DIR)lib_task.o \
        $(OUT_DIR)lib_switch.o \
        $(OUT_DIR)lib_dependent_task.o \
        $(OUT_DIR)lib_independent_task.o \
        $(OUT_DIR)lib_sched_deuce.o \
        $(OUT_DIR)lib_mutex.o \
        $(OUT_DIR)lib_sem.o 

lib_sem :task/semaphore.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sem.o $(CORE_LIB_TASK)semaphore.c

lib_task :task/task.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_task.o $(CORE_LIB_TASK)task.c

lib_mutex :task/mutex.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_mutex.o $(CORE_LIB_TASK)mutex.c

lib_independent_task :task/independent_task.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_independent_task.o $(CORE_LIB_TASK)independent_task.c

lib_dependent_task :task/dependent_task.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_dependent_task.o $(CORE_LIB_TASK)dependent_task.c

lib_sched_deuce :task/sched_deuce.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_sched_deuce.o $(CORE_LIB_TASK)sched_deuce.c

lib_switch :task/switch.asm
	$(ASM) $(ASMBFLAGS) -f elf -o $(OUT_DIR)lib_switch.o $(CORE_LIB_TASK)switch.asm



