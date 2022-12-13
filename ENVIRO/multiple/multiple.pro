TEMPLATE = app
CONFIG   += console
SOURCES  = multiple.c message.c
TARGET   = multiple
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm
include( $${MY_BASE}/common.pri )

QT +=  opengl 
