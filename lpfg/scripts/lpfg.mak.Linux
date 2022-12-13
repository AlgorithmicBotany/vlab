lsys.so: lsys.o
	g++ -O3 -std=gnu++11 -fPIC -shared -Xlinker -Bsymbolic -o lsys.so lsys.o

lsys.o: lsys.ii
	g++ -O3 -std=gnu++11 -fPIC -c lsys.ii -Wno-return-type-c-linkage

