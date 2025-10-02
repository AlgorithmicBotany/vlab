@echo off
REM not copying enviro.bin or enviro headers or oofs or lstudio.cfg

rmdir /s/q L-studio > nul
mkdir L-studio

mkdir L-studio\bin
for %%i in (Lstudio cpfg lpfg cpp) do call :CPEXEC %%i

mkdir L-studio\bin\enviro.bin
for %%i in (chiba clover ecosystem honda81 soil) do call :CPENVP %%i
copy L-studio\bin\enviro.bin\ecosystem.exe L-studio\bin\enviro.bin\ecosystemR.exe > nul
if errorlevel=1 echo Could not copy ecosystem.exe to ecosystemR.exe

xcopy /e/i Help L-studio\Help > nul
if errorlevel=1 echo Could not copy L-studio manual.
copy ..\lpfg\LPFGManual.pdf L-studio\Help\LPFGman.pdf > nul
if errorlevel=1 echo Could not copy lpfg manual.
copy ..\cpfg\CPFGManual\manual.pdf L-studio\Help\CPFGman.pdf > nul
if errorlevel=1 echo Could not copy cpfg manual.

mkdir L-studio\lpfg
mkdir L-studio\lpfg\bin
copy ..\lpfg\scripts\*.bat L-studio\lpfg\bin > nul
if errorlevel=1 echo Could not copy lpfg build scripts.
copy ..\lpfg\scripts\Makefile L-studio\lpfg\bin > nul
if errorlevel=1 echo Could not copy lpfg Makefile.
copy ..\l2c\Debug\l2c.exe L-studio\lpfg\bin > nul
if errorlevel=1 echo Could not copy l2c executable.
mkdir L-studio\lpfg\include
copy ..\lpfg\include\*.h L-studio\lpfg\include > nul
if errorlevel=1 echo Could not copy lpfg includes.

goto :eof

:CPEXEC
copy ..\%1\Debug\%1.exe L-studio\bin > nul
if errorlevel=1 echo Could not copy %1 executable.
goto :eof

:CPENVP
copy ..\ENVIRO\%1\Debug\%1.exe L-studio\bin\enviro.bin > nul
if errorlevel=1 echo Could not copy environmental program %1.
goto :eof
