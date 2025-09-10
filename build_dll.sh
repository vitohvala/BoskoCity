#!/bin/bash

LIBS='-lX11 -lGL'
INC=-I../third_party
FLAGS='-g -Wall -Wextra -O0 -fno-builtin -Wno-unused-function -Wno-unused-parameter -Wno-missing-braces -Wno-unused-variable -Wno-writable-strings -DDEBUG_BUILD' 

pushd build

clang++ ../code/game.cpp -shared -o game.so $FLAGS $INC 

popd