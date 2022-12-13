TEMPLATE = app
CONFIG   += console
SOURCES  = density3d.c octree.c message.c
TARGET   = density3d
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm image
include( $${MY_BASE}/common.pri )

QT +=  opengl 
