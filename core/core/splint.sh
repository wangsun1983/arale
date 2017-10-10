#!/bin/bash

splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include clock/sysclock.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include clock/time.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include common/sys_observer.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include input/key_dispatcher.c

