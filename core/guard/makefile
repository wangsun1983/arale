
CFLAGS += -I guard/include

CORE_LIB_GUARD 	= guard/
CORE_LIB_GUARD_MM	= guard/mm/
CORE_LIB_GUARD_FS	= guard/fs/
CORE_LIB_GUARD_TASK	= guard/task/

LINKO +=  lib_test_fs \
          lib_test_main \
					lib_test_mm \
					lib_test_mm_kmalloc \
					lib_test_mm_vmalloc \
					lib_test_mm_pmalloc \
					lib_test_task \
					lib_test_task_independent \
					lib_test_task_dependent \
					lib_test_task_sleep

OBJS	+= $(OUT_DIR)lib_test_fs.o \
         $(OUT_DIR)lib_test_main.o \
				 $(OUT_DIR)lib_test_mm.o \
				 $(OUT_DIR)lib_test_kmalloc.o \
				 $(OUT_DIR)lib_test_vmalloc.o \
				 $(OUT_DIR)lib_test_pmalloc.o \
				 $(OUT_DIR)lib_test_task.o \
				 $(OUT_DIR)lib_test_task_independent.o \
				 $(OUT_DIR)lib_test_task_dependent.o \
				 $(OUT_DIR)lib_test_task_sleep.o

lib_test_fs :guard/fs/test_fs.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_fs.o $(CORE_LIB_GUARD_FS)test_fs.c

lib_test_main :guard/test_main.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_main.o $(CORE_LIB_GUARD)test_main.c

lib_test_mm :guard/mm/test_mm.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_mm.o $(CORE_LIB_GUARD_MM)test_mm.c

lib_test_mm_kmalloc :guard/mm/test_mm_kmalloc.c
		  $(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_kmalloc.o $(CORE_LIB_GUARD_MM)test_mm_kmalloc.c

lib_test_mm_vmalloc :guard/mm/test_mm_vmalloc.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_vmalloc.o $(CORE_LIB_GUARD_MM)test_mm_vmalloc.c

lib_test_mm_pmalloc :guard/mm/test_mm_pmalloc.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_pmalloc.o $(CORE_LIB_GUARD_MM)test_mm_pmalloc.c

lib_test_task :guard/task/test_task.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_task.o $(CORE_LIB_GUARD_TASK)test_task.c

lib_test_task_independent :guard/task/test_task_independent.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_task_independent.o $(CORE_LIB_GUARD_TASK)test_task_independent.c

lib_test_task_dependent :guard/task/test_task_dependent.c
						$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_task_dependent.o $(CORE_LIB_GUARD_TASK)test_task_dependent.c

lib_test_task_sleep :guard/task/test_task_sleep.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_task_sleep.o $(CORE_LIB_GUARD_TASK)test_task_sleep.c
