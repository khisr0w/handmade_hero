@echo off

set CommonCompilerFlags=-O2 -MTd -Gm- -nologo -fp:fast -GR- -EHa- -Zo -FC -Oi -Z7 -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -DHANDMADE_WIN32=1 -DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 
set CommonLinkerFlags= -incremental:no -opt:ref  user32.lib Gdi32.lib winmm.lib	

cd ..
IF NOT EXIST bin mkdir bin
pushd bin
echo ========================================================================
REM 32-bit build
REM cl %CommonCompilerFlags% ..\src\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
del *.pdb > NUL 2> NUL
set VAR=%date:~10,4%%date:~4,2%%date:~7,2%_%time:~1,1%%time:~3,2%%time:~6,2%%time:~9,2%
REM Optimization Switches /O2
echo WAITING FOR PDB > lock.tmp
cl %CommonCompilerFlags% -I..\iaca-win64 ..\src\handmade.cpp -Fmhandmade.map -LD /link -incremental:no -opt:ref -PDB:handmade_%VAR%.pdb -EXPORT:GameUpdateAndRender -EXPORT:GameGetSoundSamples
del lock.tmp
cl %CommonCompilerFlags% ..\src\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
popd
cd src
