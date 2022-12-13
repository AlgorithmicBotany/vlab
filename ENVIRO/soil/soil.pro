TEMPLATE = app
CONFIG   += console
SOURCES  = soil2d.c matrix.c triangulate.c soil3d.c targa.c message.c
TARGET   = soil
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm image
include( $${MY_BASE}/common.pri )

QT +=  opengl 
