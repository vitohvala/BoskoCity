set -xe

LIBS='-lX11 -lGL'
INC=-I../third_party
FLAGS='-g -Wall -Wextra -O0 -fno-builtin -Wno-unused-function -Wno-unused-parameter -Wno-missing-braces -Wno-unused-variable -Wno-writable-strings' 

pushd build

clang ../code/game.cpp -shared -o game.so $FLAGS $INC -fpic 
clang ../code/untitled_linux.cpp -o linux_bosko $INC $LIBS $FLAGS 

popd 
