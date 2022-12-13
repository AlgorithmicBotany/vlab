TEMPLATE = app
CONFIG   += console
SOURCES  = takenaka.c message.c
TARGET   = takenaka
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm
include( $${MY_BASE}/common.pri )

QT +=  opengl 
