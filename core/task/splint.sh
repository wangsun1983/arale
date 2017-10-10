#!/bin/bash

splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include dependent_task.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include independent_task.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include mutex.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include sched_deuce.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include semaphore.c
splint -I ../include/ -I ../arch/x86/include/ -I ../drivers/include/ -I ../guard/include task.c


