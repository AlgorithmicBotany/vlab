@echo off
call vcvars32
IF EXIST .\makefile (
nmake /nologo /S
) ELSE (
nmake /nologo /S /f "%LPFGPATH%\bin\Makefile"
)
