TEMPLATE = lib
CONFIG  += staticlib
CONFIG -= qt
TARGET   = rayimage
SOURCES  = image.c lodepng.c
HEADERS = 
INCLUDEPATH += ../ ../.. /usr/X11/include /usr/local/include
QMAKE_LINK = $$QMAKE_LINK_C



#QT = core
macx {
  DEFINES += MACX_OPENGL_HEADERS
  LIBS += -framework Carbon 
}

MY_BASE  = ../../..
include( $${MY_BASE}/common.pri )
