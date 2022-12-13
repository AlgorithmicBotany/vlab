COMPILER=g++

lsys.dll: lsys.o
	@echo Creating lsys.dll
	$(COMPILER) -static -shared lsys.o -o lsys.dll

lsys.o: lsys.i
	@echo Compiling lsys.i
	$(COMPILER) -c lsys.i -o lsys.o
