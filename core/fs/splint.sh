#!/bin/bash

splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include fs.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include fs_dir.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include fs_file.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include fs_inode.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include fs_utils.c

