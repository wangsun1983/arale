
CFLAGS += -I guard/include

CORE_LIB_GUARD 	= guard/

LINKO +=  lib_test_fs \
          lib_test_main \
					lib_test_mm

OBJS	+= $(OUT_DIR)lib_test_fs.o \
         $(OUT_DIR)lib_test_main.o \
				 $(OUT_DIR)lib_test_mm.o \

lib_test_fs :guard/test_fs.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_fs.o $(CORE_LIB_GUARD)test_fs.c

lib_test_main :guard/test_main.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_main.o $(CORE_LIB_GUARD)test_main.c

lib_test_mm :guard/test_mm.c
			$(CC) $(CFLAGS) -o $(OUT_DIR)lib_test_mm.o $(CORE_LIB_GUARD)test_mm.c
