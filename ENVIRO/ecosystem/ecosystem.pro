TEMPLATE = app
CONFIG   += console
SOURCES  = ecosystem.c grid.c message.c
TARGET   = ecosystem
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm
include( $${MY_BASE}/common.pri )
#The following line was inserted by qt3to4
QT +=  opengl 
