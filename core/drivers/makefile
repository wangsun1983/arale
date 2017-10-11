
CORE_LIB_DRIVERS = drivers/

LINKO += lib_keyboard \
         lib_hdd \
				 lib_mouse \

OBJS +=  $(OUT_DIR)lib_keyboard.o \
         $(OUT_DIR)lib_hdd.o \
				 $(OUT_DIR)lib_mouse.o \

lib_keyboard :drivers/keyboard/keyboard.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_keyboard.o $(CORE_LIB_DRIVERS)keyboard/keyboard.c

lib_hdd :drivers/hdd/hdd.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_hdd.o $(CORE_LIB_DRIVERS)hdd/hdd.c

lib_mouse :drivers/mouse/mouse.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_mouse.o $(CORE_LIB_DRIVERS)mouse/mouse.c
