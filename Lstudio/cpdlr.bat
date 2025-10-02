copy ..\cpfg\release\cpfg.exe release
copy ..\lpfgMV\release\lpfg.exe release
copy ..\l2cMV\release\l2c.exe lpfg\bin
del /Q/F lpfg\bin\*
copy ..\lpfgMV\bin\* lpfg\bin
copy ..\l2cMV\release\l2c.exe lpfg\bin
del /Q/F lpfg\include\*
copy ..\lpfgMV\include\* lpfg\include

