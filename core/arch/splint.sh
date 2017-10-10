#!/bin/bash

splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include x86/cmos.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include x86/cpu.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include x86/dma.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include x86/except.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include x86/i8253.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include x86/i8259.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include x86/idt.c

