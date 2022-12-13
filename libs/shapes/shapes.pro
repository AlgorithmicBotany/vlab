TEMPLATE = lib
CONFIG  += staticlib opengl
TARGET   = shapes
SOURCES  = bsurface.cpp matrix.cpp
HEADERS  = bsurface.h matrix.h point.h
INCLUDEPATH += ../../lpfg

MY_BASE  = ../..
MY_LIBS  = 
include( $${MY_BASE}/common.pri )


QT +=  opengl
