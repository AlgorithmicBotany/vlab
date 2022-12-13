TEMPLATE = lib
CONFIG  += staticlib
CONFIG -= qt
TARGET   = raysurf
SOURCES  = atmosphere.c fog.c fogdeck.c mist.c surface.c surfshade.c
HEADERS = 
INCLUDEPATH += ../ ../.. /usr/X11/include

QMAKE_LINK = $$QMAKE_LINK_C
#QT = core
macx {
  DEFINES += MACX_OPENGL_HEADERS
  LIBS += -framework Carbon
} else {
  DEFINES += HUGE=3.40282347e+38f EPSILON=0.00001
}

MY_BASE  = ../../..
include( $${MY_BASE}/common.pri )
