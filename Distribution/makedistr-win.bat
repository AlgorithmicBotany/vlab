@echo off
REM not copying oofs or all of enviro.bin

set USEATE=

if defined TGTCOMPILER (
  goto :compilerDefined
) else (
  set TGTCOMPILER=VC
)

:compilerDefined
if %TGTCOMPILER%==ATE set USEATE=1 & set TGTCOMPILER=MinGW
echo Target compiler is %TGTCOMPILER%

if defined USEATE echo Assembling ATE version of L-studio.

for /f %%a in ('..\version\Debug\version.exe') do set VERNUM=%%a
for /f %%a in ('..\version\Debug\version.exe -b') do set BUILDNUM=%%a



if defined USEATE (
  set TGTDIR=L-studio-ATE-%VERNUM%-%BUILDNUM%
  set TGTZIP=L-studio-ATE-%VERNUM%-%BUILDNUM%.zip
) else (
  set TGTDIR=L-studio-%VERNUM%-%BUILDNUM%
  set TGTZIP=L-studio-%VERNUM%-%BUILDNUM%.zip
)

rmdir /s/q %TGTDIR% 2> nul
mkdir %TGTDIR%

REM ******************
REM Executables
REM ******************
mkdir %TGTDIR%\bin
for %%i in (Lstudio cpfg lpfg vlabcpp) do call :CPEXEC %1 %%i

mkdir %TGTDIR%\bin\enviro.bin
for %%i in (chiba clover ecosystem honda81 soil MonteCarlo QuasiMC) do call :CPENVP %1 %%i
copy %TGTDIR%\bin\enviro.bin\ecosystem.exe %TGTDIR%\bin\enviro.bin\ecosystemR.exe > nul
if errorlevel=1 echo Could not copy ecosystem.exe to ecosystemR.exe


REM ******************
REM Utilities for making environmental programs
REM ******************
mkdir %TGTDIR%\enviro
copy ..\libs\comm\%1\comm_lib.lib %TGTDIR%\enviro > nul
if errorlevel=1 echo Could not copy comm_lib.lib
copy ..\libs\comm\message.c %TGTDIR%\enviro > nul
if errorlevel=1 echo Could not copy comm_lib.h
copy ..\libs\comm\comm_lib.h %TGTDIR%\enviro > nul
if errorlevel=1 echo Could not copy message.c


REM ******************
REM User's manuals
REM ******************
xcopy /e/i ..\Lstudio\Help %TGTDIR%\Help > nul
if errorlevel=1 echo Could not copy L-studio manual.
copy ..\lpfg\LPFGManual.pdf %TGTDIR%\Help\LPFGman.pdf > nul
if errorlevel=1 echo Could not copy lpfg manual.
copy ..\cpfg\CPFGManual\manual.pdf %TGTDIR%\Help\CPFGman.pdf > nul
if errorlevel=1 echo Could not copy cpfg manual.


REM ******************
REM Example objects
REM ****************** 
xcopy /e/i oofs %TGTDIR%\oofs > nul
if errorlevel=1 echo Could not copy oofs.


REM ******************
REM OpenGL shaders
REM ******************
xcopy /e/i ..\lpfg\shaders %TGTDIR%\shaders > nul
if errorlevel=1 echo Could not copy shaders.


REM ******************
REM LPFG stuff
REM ******************
mkdir %TGTDIR%\lpfg
mkdir %TGTDIR%\lpfg\bin
copy ..\l2c\%1\l2c.exe %TGTDIR%\lpfg\bin > nul
if errorlevel=1 echo Could not copy l2c executable.

xcopy /e/i ..\lpfg\scripts\VC %TGTDIR%\lpfg\bin\VC > nul
xcopy /e/i ..\lpfg\scripts\MinGW %TGTDIR%\lpfg\bin\MinGW > nul
copy %TGTDIR%\lpfg\bin\%TGTCOMPILER%\*.* %TGTDIR%\lpfg\bin > nul
if defined USEATE copy ..\lpfg\scripts\MinGW.bat %TGTDIR%\bin > nul
if errorlevel=1 echo Could not copy lpfg build scripts

if defined USEATE xcopy /e/i MinGW %TGTDIR%\MinGW > nul
if errorlevel=1 echo Could not copy compiler from directory MinGW

mkdir %TGTDIR%\lpfg\include
copy ..\lpfg\include\*.h %TGTDIR%\lpfg\include > nul
if errorlevel=1 echo Could not copy lpfg includes.

copy ..\libs\input\WinTablet\%1\WinTablet.dll %TGTDIR%\bin
REM copy ..\libs\input\Connection3D\%1\Connection3D.dll %TGTDIR%\bin
if errorlevel=1 echo Could not copy input device drivers.

REM ******************
REM lstudio.cfg
REM ******************
copy ..\Lstudio\lstudio.cfg %TGTDIR% > nul
if errorlevel=1 echo Could not copy lstudio.cfg.


REM ******************
REM Zip it all up
REM ******************
echo Zipping directory %TGTDIR% to %TGTZIP%...
del /f/q %TGTZIP% 2> nul
REM zip -9 -r %TGTZIP% %TGTDIR% > nul
7z a -tzip %TGTZIP% %TGTDIR% > nul
if errorlevel 1 echo Could not zip up the distribution.

REM ******************
REM Clean up after ourselves
REM ******************
set USEATE=
set VERNUM=
set BUILDNUM=
set TGTDIR=
set TGTZIP=

goto :eof

:CPEXEC
copy ..\%2\%1\%2.exe %TGTDIR%\bin > nul
if errorlevel=1 echo Could not copy %2 executable.
goto :eof

:CPENVP
copy ..\ENVIRO\%2\%1\%2.exe %TGTDIR%\bin\enviro.bin > nul
if errorlevel=1 echo Could not copy environmental program %2.
goto :eof
