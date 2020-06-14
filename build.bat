cls
@echo off

set CommonCompilerFlags= -MT -Gm- -nologo -GR- -EHa- -Od -Oi -FC -Z7 -WX -W4 -wd4201 -wd4100 -wd4189 -wd4456 -DHANDMADE_WIN32=1 -DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 
set CommonLinkerFlags= -incremental:no -opt:ref  user32.lib Gdi32.lib winmm.lib	

cd ..
IF NOT EXIST bin mkdir bin
pushd bin
echo ========================================================================
REM 32-bit build
REM cl %CommonCompilerFlags% ..\src\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%
REM 64-bit build
cl %CommonCompilerFlags% ..\src\handmade.cpp -Fmhandmade.map /LD /link /EXPORT:GameUpdateAndRender /EXPORT:GameGetSoundSamples
cl %CommonCompilerFlags% ..\src\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
popd
