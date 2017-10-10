#!/bin/bash

splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include libc/arraylist.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include libc/bitmap.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include libc/fifo_list.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include libc/kerror.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include libc/klibc.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include libc/linkqueue.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include libc/rbtree.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include libc/stack.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include libc/trigger.c

