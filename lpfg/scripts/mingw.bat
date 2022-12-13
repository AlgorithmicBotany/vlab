REM ** Define variables for MinGW

REM A "proper" installation of MinGW will set MINGWDIR
if defined MINGWDIR (
  set MINGW=%MINGWDIR%
) else (
  set MINGW=%LPFGPATH%\..\MinGW
)

if not exist %MINGW% set /p MINGW="MINGW path invalid(%MINGW%), enter MINGW path: "

set PATH=%MINGW%\bin;%PATH%
set MINGWMAKE=%MINGW%\bin\mingw32-make
