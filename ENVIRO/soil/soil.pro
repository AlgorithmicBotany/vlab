TEMPLATE = app
CONFIG   += console
SOURCES  = soil2d.c matrix.c triangulate.c soil3d.c targa.c message.c lodepng.c
TARGET   = soil
VPATH += ../../libs/comm

INCLUDEPATH += ../../libs
MY_BASE  = ../..
MY_LIBS  = image comm 
include( $${MY_BASE}/common.pri )
