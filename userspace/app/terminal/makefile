USERSPACE_APP_TERMINAL = ../userspace/app/terminal/

LINKO += lib_app_terminal \


OBJS += $(OUT_DIR)lib_app_terminal.o \

lib_app_terminal :../userspace/app/terminal/terminal_main.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_app_terminal.o $(USERSPACE_APP_TERMINAL)terminal_main.c
