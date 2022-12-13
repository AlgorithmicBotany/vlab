TEMPLATE = app
CONFIG   += console
SOURCES  = shadowpyramid.cpp message.c
TARGET   = shadowpyramid
VPATH += ../../libs/comm
INCLUDEPATH += ../../lpfg/include

MY_BASE  = ../..
MY_LIBS  = comm
include( $${MY_BASE}/common.pri )
