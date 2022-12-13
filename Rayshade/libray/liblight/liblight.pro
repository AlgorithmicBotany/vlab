TEMPLATE = lib
LANGUAGE = C
CONFIG  += staticlib
CONFIG -= qt
TARGET   = raylight
SOURCES  = light.c extended.c infinite.c jittered.c point.c shadow.c spot.c
INCLUDEPATH += ../ ../.. /usr/X11/include
#HEADER = ../libobj/geom.h

QMAKE_LINK = $$QMAKE_LINK_C
#QT = core
macx {
  DEFINES += MACX_OPENGL_HEADERS
  LIBS += -framework Carbon
}

MY_BASE  = ../../..
MY_LIBS =
include( $${MY_BASE}/common.pri )

