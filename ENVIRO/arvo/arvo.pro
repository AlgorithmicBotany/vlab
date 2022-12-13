TEMPLATE = app
CONFIG   += console
SOURCES  = arvo.c matrix.c scene3d.c message.c
TARGET   = arvo
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm image
include( $${MY_BASE}/common.pri )

QT +=  opengl 
