TEMPLATE = app
CONFIG   += console
SOURCES  = MonteCarlo.c matrix.c scene3d.c sky.c message.c
TARGET   = MonteCarlo
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm
include( $${MY_BASE}/common.pri )

QT +=  opengl 
