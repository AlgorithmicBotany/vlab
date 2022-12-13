TEMPLATE = app
CONFIG   += console
SOURCES  = ornament.c targa.c message.c
TARGET   = ornament
VPATH += ../../libs/comm ../../libs/image

MY_BASE  = ../..
MY_LIBS  = comm image
include( $${MY_BASE}/common.pri )
#The following line was inserted by qt3to4
QT +=  opengl 
