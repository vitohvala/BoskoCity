@echo off
if not exist build mkdir build
pushd build

set inc=-I "../third_party/"
set CommonCompilerFlags=-nologo -FC -WX -W4 -wd4201 -wd4576 -wd4101 -wd4505 -wd4100 -wd4189 -MT -Oi -Od -GR- -Gm- -EHa- -Zi /DDEBUG
set export=-EXPORT:game_update

cl %CommonCompilerFlags% ../code/game.cpp %inc% /INCREMENTAL:NO -LD /link %export%

popd
