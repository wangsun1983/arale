
CORE_LIB_MM = mm/

LINKO += lib_pmm \
         lib_vmm \
         lib_mm \
         lib_mmzone \
         lib_coalition_allocator \
         lib_fragment_allocator \
         lib_vm_allocator \
         lib_cache_allocator \
         lib_gdt \


OBJS += $(OUT_DIR)lib_gdt.o \
        $(OUT_DIR)lib_mm.o \
        $(OUT_DIR)lib_mmzone.o \
        $(OUT_DIR)lib_coalition_allocator.o \
        $(OUT_DIR)lib_fragment_allocator.o \
        $(OUT_DIR)lib_vm_allocator.o \
        $(OUT_DIR)lib_cache_allocator.o \
        $(OUT_DIR)lib_pmm.o \
        $(OUT_DIR)lib_vmm.o \

lib_pmm :mm/pmm.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_pmm.o $(CORE_LIB_MM)pmm.c

lib_vmm :mm/vmm.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_vmm.o $(CORE_LIB_MM)vmm.c

lib_gdt :mm/gdt.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_gdt.o $(CORE_LIB_MM)gdt.c

lib_mm :mm/mm.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_mm.o $(CORE_LIB_MM)mm.c

lib_fragment_allocator :mm/fragment_allocator.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_fragment_allocator.o $(CORE_LIB_MM)fragment_allocator.c

lib_mmzone :mm/mmzone.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_mmzone.o $(CORE_LIB_MM)mmzone.c

lib_coalition_allocator :mm/coalition_allocator.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_coalition_allocator.o $(CORE_LIB_MM)coalition_allocator.c

lib_vm_allocator :mm/vm_allocator.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_vm_allocator.o $(CORE_LIB_MM)vm_allocator.c

lib_cache_allocator :mm/cache_allocator.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_cache_allocator.o $(CORE_LIB_MM)cache_allocator.c





