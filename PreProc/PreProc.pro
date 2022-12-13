TEMPLATE = app
CONFIG  += qt
SOURCES  = HashFn.cpp
LEXSOURCES = scanline.l
TARGET   = preproc

MY_BASE  = ..
MY_LIBS  = 
QMAKE_CXXFLAGS += -Wno-deprecated-register
include( $${MY_BASE}/common.pri )

