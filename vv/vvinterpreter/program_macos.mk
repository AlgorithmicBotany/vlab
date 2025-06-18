# System portion of makefile for models
#
RESOURCES=$(shell vvinterpreter -resources)
QTDIR=$(shell vvinterpreter -qtdir)
VVLIBDIR=$(RESOURCES)/libs
VVHEADERS=$(RESOURCES)/include
CXX = clang++
CXXFLAGS = -O2 -O3 -ffast-math -fPIC -std=gnu++11 -ARCH -Wno-deprecated-declarations -Wno-return-type-c-linkage
INCDIRS = -I'$(QTDIR)/QtCore.framework/Headers' -I'$(QTDIR)/QtOpenGL.framework/Headers' -I'$(QTDIR)/QtXml.framework/Headers' \
	  -I'$(QTDIR)/QtGui.framework/Headers' -I'$(VVHEADERS)' -F'$(VVLIBDIR)' -F$(QTDIR)

LIBS_MacOS = -F$(QTDIR) -framework QtXml -framework QtOpenGL -framework QtCore -framework QtGui  -framework OpenGL
LIBS = -L'$(VVLIBDIR)' -lvv $(LIBS_MacOS)

%.vv: %.o
	$(CXX) $(CXXFLAGS) $(INCDIRS) $(OBJS) $(LIBDIRS) $(LIBS) -dynamiclib -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCDIRS) -c -o $@ $< 

%.cpp: %.vvp
	vv2cpp $< $@

