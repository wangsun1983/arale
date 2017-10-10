#!/bin/bash

splint -I include/ -I arch/x86/include/ -I drivers/include/ -I guard/include start.c

