@echo off
call vcvars32
cl /nologo @clopt /E /TP %1 > %2
