TEMPLATE = app
CONFIG   += console
SOURCES  = radiosity.c scene3d.c sky.c matrix.c message.c
TARGET   = radiosity
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm
include( $${MY_BASE}/common.pri )

QT +=  opengl
