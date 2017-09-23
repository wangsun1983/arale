
CORE_LIB_FS = fs/

LINKO += lib_fs \
         lib_fs_utils \
         lib_fs_dir \
         lib_fs_inode \
         lib_fs_file \


OBJS += $(OUT_DIR)lib_fs.o \
        $(OUT_DIR)lib_fs_utils.o \
        $(OUT_DIR)lib_fs_dir.o \
        $(OUT_DIR)lib_fs_inode.o \
        $(OUT_DIR)lib_fs_file.o 

lib_fs :fs/fs.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_fs.o $(CORE_LIB_FS)fs.c

lib_fs_utils :fs/fs_utils.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_fs_utils.o $(CORE_LIB_FS)fs_utils.c

lib_fs_file :fs/fs_file.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_fs_file.o $(CORE_LIB_FS)fs_file.c

lib_fs_dir :fs/fs_dir.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_fs_dir.o $(CORE_LIB_FS)fs_dir.c

lib_fs_inode :fs/fs_inode.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_fs_inode.o $(CORE_LIB_FS)fs_inode.c



