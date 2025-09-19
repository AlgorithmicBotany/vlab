# System portion of makefile for models
#
RESOURCES=$(shell vvinterpreter -resources)

VVLIBDIR=$(RESOURCES)/../lib
VVHEADERS=$(RESOURCES)/../include/vv

CXX = g++
CXXFLAGS = -O2 -O3 -ffast-math -fPIC -std=gnu++11
INCDIRS = -I'$(VVHEADERS)' -F'$(VVLIBDIR)'
LIBS = -L'$(VVLIBDIR)' -lvv

%.vv: %.o
	$(CXX) $(CXXFLAGS) $(INCDIRS) $(OBJS) $(LIBDIRS) $(LIBS) -shared -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIRS) -c -o $@ $< 

%.cpp: %.vvp
	vvp2cpp $< $@
