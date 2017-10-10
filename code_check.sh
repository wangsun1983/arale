#!/bin/bash

#we should install splint

cd core
./splint.sh

cd ..
cd core/arch
./splint.sh

cd ../../
cd core/core
./splint.sh

cd ../../
cd core/drivers
./splint.sh

cd ../../
cd core/fs
./splint.sh

cd ../../
cd core/lib
./splint.sh

cd ../../
cd core/mm
./splint.sh

cd ../../
cd core/task
./splint.sh
