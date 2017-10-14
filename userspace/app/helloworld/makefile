USERSPACE_APP_HELLOWORLD = ../userspace/app/helloworld/

LINKO += lib_app_helloworld \


OBJS += $(OUT_DIR)lib_app_helloworld.o \

lib_app_helloworld :../userspace/app/helloworld/helloworld_main.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_app_helloworld.o $(USERSPACE_APP_HELLOWORLD)helloworld_main.c
