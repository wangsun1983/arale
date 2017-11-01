USERSPACE_GUARD = ../userspace/guard/
USERSPACE_GUARD_IPC = ../userspace/guard/ipc/

CFLAGS += -I ../userspace/guard/include/

LINKO += lib_user_test_ipc \
         lib_user_test_main

OBJS += $(OUT_DIR)lib_user_test_ipc.o \
        $(OUT_DIR)lib_user_test_main.o

lib_user_test_ipc :../userspace/guard/ipc/user_test_ipc.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_user_test_ipc.o $(USERSPACE_GUARD_IPC)user_test_ipc.c

lib_user_test_main :../userspace/guard/user_test_main.c
		$(CC) $(CFLAGS) -o $(OUT_DIR)lib_user_test_main.o $(USERSPACE_GUARD)user_test_main.c
