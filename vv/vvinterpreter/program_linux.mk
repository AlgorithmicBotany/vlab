# System portion of makefile for models
#
CXX = g++
CXXFLAGS = -DQT3_SUPPORT -O3 -ffast-math
INCDIRS = -I'$(QTDIR)/include' -I'$(QTDIR)/include/Qt' -I'$(QTDIR)/include/QtGui' -I'$(QTDIR)/include/Qt3Support' \
   -I'$(VVDIR)/include/vvlib'

LIBS_Linux = -lvvlib
LIBS = -L'$(VVDIR)/lib' $(LIBS_Linux)

%.vv: %.o
	$(CXX) -shared -o $@ $(OBJS) $(LIBDIRS) $(LIBS)

%.o: %.cpp
	$(CXX) -fpic -c -o $@ $< $(INCDIRS) $(CXXFLAGS)

%.cpp: %.vvp
	vvp2cpp $< $@
