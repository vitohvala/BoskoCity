#!/bin/bash
set -xe

LIBS='-lX11 -lGL'
INC=-I./third_party
FLAGS='-g -Wall -Wextra -O0 -fno-builtin -Wno-unused-function -Wno-unused-parameter -Wno-missing-braces -Wno-unused-variable -Wno-writable-strings -DDEBUG_BUILD'

#pushd build

g++ ./code/game.cpp -shared -o build/game.so $FLAGS $INC
g++ ./code/untitled_linux.cpp -o build/linux_bosko $INC $LIBS $FLAGS

#popd
