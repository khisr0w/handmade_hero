@echo off

set CommonCompilerFlags= -MTd -Gm- -nologo -fp:fast -GR- -EHa- -Od -FC -Oi -Z7 -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -DHANDMADE_WIN32=1 -DHANDMADE_SLOW=1 -DHANDMADE_INTERNAL=1 
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
cl %CommonCompilerFlags% ..\src\handmade.cpp -Fmhandmade.map /LD /link -incremental:no /PDB:handmade_%VAR%.pdb -EXPORT:GameUpdateAndRender -EXPORT:GameGetSoundSamples
del lock.tmp
cl %CommonCompilerFlags% ..\src\win32_handmade.cpp -Fmwin32_handmade.map /link %CommonLinkerFlags%
popd
cd src


REM Suffix to MSVC

REM 01 - Zi (build debug information, things like the obj file and more).
REM 02 - FC (gives full path to a cpp file, for example when using it in errors).
REM 03 - cl /? | less (man page for the compiler)
REM 04 - F<num> (set stack size)
REM 05 - O2 (set optimization on)
REM 06 - D<macro>=1 (Compile Define a Macro)
REM 07 - Wall (Enable All warnings, additionally we can add W4 W3 W2 these are all warning levels)
REM 08 - WX (treat warnings as errors and will stop the compilation)
REM 09 - wd<warning code> (it will mask out a specific kind of warning)
REM 10 - Z7 (The older version of Zi, it works nicer with builds and multithreaded works) 
REM 11 - Oi (turn on intrinsics of the compiler, for things like sinf; given that the compiler has an intrinsic for that)
REM 12 - GR- (Disable runtime type information used in C++)
REM 13 - EHa- (turn off exception handling)
REM 14 - MD (Use the dll for C runtime library, may become problematic, since some versions of windows won't have some components)
REM 15 - MT (Packages all the C runtime library into our executable)
REM 16 - MTd (Packages the debug version of the C runtime library into our executable)
REM 17 - Gm- (turn off incremental rebuild)
REM 18 - Fm<name of map file> (It will create a map file, it lists all functions and everything in your exe in one file)
REM 19 - Od (turn off all optimization)
REM 20 - LD (build a .dll file when trying to make DLLs to export functions)
REM 21 - fp:<precise | except[-] | fast | strict> (Specifies floating-point behavior in source code file)
REM 22 - 


REM 01 - /link (You can put linker options after this)
REM 02 - subsystem:windows,5.1 (Linker options that will allow you to run your exe in Windows XP)
REM 03 - opt:ref (Do not put anything, import wise, if no piece of code uses it. It will get rid of extra stuff that are imported from the dll libraries)
REM 04 - /DLL (builds a dll file, doesn't seem to be that imnportant in making dll, /LD will do the job)
REM 05 - EXPORT:<name of function> (specify the functions that you want to export as part of the DLL file)
REM 06 - incremental:no (disables the incremental linker, won't be necessary for debug)
REM 07 - PDB:<name of pdb> (Creates a specifically named PDB file for debugging)
