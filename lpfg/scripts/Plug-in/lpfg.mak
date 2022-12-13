lsys.so: lsys.o
	g++ -framework Accelerate -dynamiclib -o lsys.so lsys.o

lsys.o: lsys.ii
	g++ -c lsys.ii

