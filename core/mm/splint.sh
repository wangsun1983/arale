#!/bin/bash

splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include cache_allocator.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include coalition_allocator.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include fragment_allocator.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include gdt.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include mm.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include mmzone.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include pmm.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include vm_allocator.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include vmm.c


