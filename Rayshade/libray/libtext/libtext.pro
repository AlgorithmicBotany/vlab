TEMPLATE = lib
CONFIG  += staticlib
CONFIG -= qt
TARGET   = raytext
SOURCES  =  blotch.c bump.c checker.c cloud.c fbm.c fbmbump.c gloss.c \
	 imagetext.c mapping.c marble.c mount.c noise.c sky.c stripe.c \
	 textaux.c texture.c windy.c wood.c
HEADERS = 
INCLUDEPATH += ../ ../.. /usr/X11/include
CONFIG += create_prl link_prl
QMAKE_LINK = $$QMAKE_LINK_C
DEPENDPATH = ../../../.librairies


#QT = core
macx {
  DEFINES += MACX_OPENGL_HEADERS
  LIBS += -framework Carbon
}

MY_BASE  = ../../..
include( $${MY_BASE}/common.pri )
