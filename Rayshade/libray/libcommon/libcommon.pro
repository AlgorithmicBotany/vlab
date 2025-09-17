TEMPLATE = lib
LANGUAGE = C
CONFIG  += staticlib
CONFIR -= qt
TARGET   = raycommon
SOURCES  = memory.c expr.c transform.c rotate.c sampling.c scale.c translate.c \
	vecmath.c xform.c
HEADERS = 
INCLUDEPATH += ../.. /usr/X11/include

QMAKE_LINK = $$QMAKE_LINK_C
#QT = core
macx {
  DEFINES += MACX_OPENGL_HEADERS
  LIBS += -framework Carbon
}

MY_BASE  = ../../..
include( $${MY_BASE}/common.pri )
