TEMPLATE = app
CONFIG   += console
SOURCES  = honda81.c message.c
TARGET   = honda81
VPATH += ../../libs/comm

MY_BASE  = ../..
MY_LIBS  = comm
include( $${MY_BASE}/common.pri )
#The following line was inserted by qt3to4
QT +=  opengl 
