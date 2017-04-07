#!/bin/bash

OUT_DIR="out"
rm -rf $OUT_DIR
mkdir $OUT_DIR
mkdir $OUT_DIR/core

sudo chmod 777 $OUT_DIR 
sudo chmod 777 $OUT_DIR/core

BOOT_DIR="boot"
BOOT_SRC=$BOOT_DIR/boot.asm
BOOT_INCLUDE=$BOOT_DIR/inc/

#make out directory
mkdir $OUT_DIR/tmp

#build boot1
rm $BOOT_DIR/boot.bin
rm $BOOT_DIR/boot.img
rm $BOOT_DIR/diska.img
nasm -o $OUT_DIR/boot.bin $BOOT_DIR/stage1.asm -I $BOOT_INCLUDE

#build boot2
nasm -o $OUT_DIR/STAGE2.SYS $BOOT_DIR/stage2.asm -I $BOOT_INCLUDE

#build core
make -C core 
#objcopy -O binary -R .note -R .comment -S $OUT_DIR/core/CORESE.SYS $OUT_DIR/core/CORESE2.SYS

#make boot.img
dd if=/dev/zero of=$OUT_DIR/diska.img bs=512 count=2880
dd if=$OUT_DIR/boot.bin of=$OUT_DIR/boot.img bs=512 count=1
dd if=$OUT_DIR/diska.img of=$OUT_DIR/boot.img skip=1 seek=1 bs=512 count=2879
rm $OUT_DIR/diska.img
rm $OUT_DIR/boot.bin

#mount FAT12
sudo losetup /dev/loop1 $OUT_DIR/boot.img

sudo chmod 777 $OUT_DIR/tmp
sudo mount /dev/loop1 $OUT_DIR/tmp
sudo cp $OUT_DIR/STAGE2.SYS $OUT_DIR/tmp
sudo cp $OUT_DIR/core/CORESE.SYS $OUT_DIR/tmp

sleep 1

sudo umount $OUT_DIR/tmp
sudo losetup -d /dev/loop1

cp $OUT_DIR/boot.img simulator/


