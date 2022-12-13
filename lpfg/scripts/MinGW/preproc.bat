@echo off
call mingw.bat
addClopt %MINGW%\bin\g++ -x c++ -E -I%LPFGPATH%\include %1 -o %2
