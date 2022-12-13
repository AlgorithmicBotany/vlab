@echo off
call mingw.bat
IF EXIST .\makefile (
  IF EXIST .\makefile.ate (
    %MINGWMAKE% --makefile=.\makefile.ate -e
  ) ELSE (
    START CMD /C "ECHO A local makefile for (non-ATE) L-studio has been detected. && ECHO Please provide a makefile in the object's directory called Makefile.ATE (e.g., \L-studio-ATE\lpfg\bin\Makefile), which is compatible with L-studio-ATE. && ECHO Or if the makefile is not needed, please delete it. && PAUSE"
  )
) ELSE (
%MINGWMAKE% --makefile=%LPFGPATH%\bin\Makefile -e
)
