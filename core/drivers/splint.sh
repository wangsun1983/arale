#!/bin/bash

splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include floppy/floppy.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include hdd/hdd.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include keyboard/keyboard.c

