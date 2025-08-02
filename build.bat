@echo off
if not exist build mkdir build
pushd build

set inc=-I "../third_party/"
set srcs=../code/untitled_win32.cpp
set CommonCompilerFlags=-nologo -FC -WX -W4 -wd4702 -wd4201 -wd4576 -wd4042 -wd4101 -wd4505 -wd4100 -wd4189 -MT -Oi -Od -GR- -Gm- -EHa- -Zi /DDEBUG_BUILD
set Libs=user32.lib gdi32.lib winmm.lib opengl32.lib

cl %CommonCompilerFlags% ../code/game.cpp %inc% -LD /link -EXPORT:game_update -incremental:no
cl %CommonCompilerFlags% %srcs% /Fe:bosko.exe  %inc% /INCREMENTAL:NO %Libs%

rem clang++ %srcs% -o bosko_clang.exe -I../third_party/ -Wno-writable-strings -Wno-microsoft-cast -Wno-unused-value

rem gcc %srcs% -o bosko_gcc.exe -I../third_party/

popd