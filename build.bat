clear
@echo off
cd ..
mkdir bin
pushd bin
cl -Zi ..\src\win32_handmade.cpp user32.lib Gdi32.lib
popd
