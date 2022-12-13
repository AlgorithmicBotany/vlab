TEMPLATE = app
CONFIG   += console
SOURCES  = density.c targa.c message.c
TARGET   = density
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm image
include( $${MY_BASE}/common.pri )

QT +=  opengl
