clear
@echo off
cd ..
IF NOT EXIST bin mkdir bin
pushd bin
echo  
echo ========================================================================
echo  
cl -MT -Gm- -nologo -GR- -EHa- -Od -Oi -FC -Z7 -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -DHANDMADE_WIN32=1 -DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 -Fmwin32_handmade.map ..\src\win32_handmade.cpp /link -opt:ref -subsystem:windows,5.1 user32.lib Gdi32.lib
popd
