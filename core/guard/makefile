
CFLAGS += -I guard/include

CORE_LIB_GUARD 	= guard/
CORE_LIB_GUARD_MM	= guard/mm/
CORE_LIB_GUARD_FS	= guard/fs/
CORE_LIB_GUARD_TASK	= guard/task/
CORE_LIB_GUARD_LIBC	= guard/libc/

LINKO +=  lib_test_fs \
          lib_test_main \
          lib_test_mm \
          lib_test_mm_kmalloc \
          lib_test_mm_vmalloc \
          lib_test_mm_pmalloc \
          lib_test_task \
          lib_test_task_independent \
          lib_test_task_dependent \
          lib_test_task_sleep \
          lib_test_task_mutex \
					lib_test_task_semaphore \
          lib_test_mm_statistic \
					lib_test_mm_reclaim \
					lib_test_libc_spin_lock \
					lib_test_libc_fifolist \
					lib_test_libc_stack \
					lib_test_libc \

OBJS	+= $(OUT_DIR)lib_test_fs.o \
         $(OUT_DIR)lib_test_main.o \
         $(OUT_DIR)lib_test_mm.o \
         $(OUT_DIR)lib_test_kmalloc.o \
         $(OUT_DIR)lib_test_vmalloc.o \
         $(OUT_DIR)lib_test_pmalloc.o \
         $(OUT_DIR)lib_test_task.o \
         $(OUT_DIR)lib_test_task_independent.o \
         $(OUT_DIR)lib_test_task_dependent.o \
         $(OUT_DIR)lib_test_task_sleep.o \
         $(OUT_DIR)lib_test_task_mutex.o \
				 $(OUT_DIR)lib_test_task_semaphore.o \
         $(OUT_DIR)lib_test_mm_statistic.o \
				 $(OUT_DIR)lib_test_mm_reclaim.o \
				 $(OUT_DIR)lib_test_libc_spin_lock.o \
				 $(OUT_DIR)lib_test_libc_fifolist.o \
				 $(OUT_DIR)lib_test_libc_stack.o \
				 $(OUT_DIR)lib_test_libc.o \

lib_test_fs :guard/fs/test_fs.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_fs.o $(CORE_LIB_GUARD_FS)test_fs.c

lib_test_main :guard/test_main.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_main.o $(CORE_LIB_GUARD)test_main.c

lib_test_mm :guard/mm/test_mm.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_mm.o $(CORE_LIB_GUARD_MM)test_mm.c

lib_test_mm_statistic :guard/mm/test_mm_statistic.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_mm_statistic.o $(CORE_LIB_GUARD_MM)test_mm_statistic.c

lib_test_mm_kmalloc :guard/mm/test_mm_kmalloc.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_kmalloc.o $(CORE_LIB_GUARD_MM)test_mm_kmalloc.c

lib_test_mm_vmalloc :guard/mm/test_mm_vmalloc.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_vmalloc.o $(CORE_LIB_GUARD_MM)test_mm_vmalloc.c

lib_test_mm_pmalloc :guard/mm/test_mm_pmalloc.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_pmalloc.o $(CORE_LIB_GUARD_MM)test_mm_pmalloc.c

lib_test_mm_reclaim :guard/mm/test_mm_reclaim.c
		$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_mm_reclaim.o $(CORE_LIB_GUARD_MM)test_mm_reclaim.c

lib_test_task :guard/task/test_task.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_task.o $(CORE_LIB_GUARD_TASK)test_task.c

lib_test_task_independent :guard/task/test_task_independent.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_task_independent.o $(CORE_LIB_GUARD_TASK)test_task_independent.c

lib_test_task_dependent :guard/task/test_task_dependent.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_task_dependent.o $(CORE_LIB_GUARD_TASK)test_task_dependent.c

lib_test_task_semaphore :guard/task/test_task_semaphore.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_task_semaphore.o $(CORE_LIB_GUARD_TASK)test_task_semaphore.c

lib_test_task_mutex :guard/task/test_task_mutex.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_task_mutex.o $(CORE_LIB_GUARD_TASK)test_task_mutex.c

lib_test_task_sleep :guard/task/test_task_sleep.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_task_sleep.o $(CORE_LIB_GUARD_TASK)test_task_sleep.c

lib_test_libc_spin_lock :guard/libc/test_libc_spin_lock.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_libc_spin_lock.o $(CORE_LIB_GUARD_LIBC)test_libc_spin_lock.c

lib_test_libc_fifolist :guard/libc/test_libc_fifolist.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_libc_fifolist.o $(CORE_LIB_GUARD_LIBC)test_libc_fifolist.c

lib_test_libc_stack :guard/libc/test_libc_stack.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_libc_stack.o $(CORE_LIB_GUARD_LIBC)test_libc_stack.c

lib_test_libc :guard/libc/test_libc.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_libc.o $(CORE_LIB_GUARD_LIBC)test_libc.c
