
#CORE DISPLAY
CORE_LIB_GRAPHIC = graphic/

LINKO += lib_display \
         lib_font 

OBJS += $(OUT_DIR)lib_graphic.o \
        $(OUT_DIR)lib_font.o  

lib_display :graphic/graphic.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_graphic.o $(CORE_LIB_GRAPHIC)graphic.c

lib_font :graphic/font.c
	$(CC) $(CFLAGS) -o $(OUT_DIR)lib_font.o $(CORE_LIB_GRAPHIC)font.c




