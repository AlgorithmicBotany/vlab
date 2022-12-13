TEMPLATE = lib
CONFIG  += staticlib
TARGET   = image
SOURCES  = image.c

MY_BASE  = ../..
MY_LIBS  = 
include( $${MY_BASE}/common.pri )
#The following line was inserted by qt3to4
