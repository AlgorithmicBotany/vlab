TEMPLATE = app
CONFIG   += console
SOURCES  = clover.c targa.c message.c lodepng.c
TARGET   = clover
VPATH += ../../libs/comm ../../libs/misc

MY_BASE  = ../..
MY_LIBS  = comm image
include( $${MY_BASE}/common.pri )

QT +=  opengl 
