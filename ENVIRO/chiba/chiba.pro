TEMPLATE = app
CONFIG   += console
SOURCES  = chiba.c sphere.c message.c
TARGET   = chiba
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm
include( $${MY_BASE}/common.pri )

QT +=  opengl 
