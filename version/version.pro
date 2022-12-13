TEMPLATE = app
CONFIG  += qt
SOURCES  = version.cpp
TARGET   = version

MY_BASE  = ..
MY_LIBS  = misc
include( $${MY_BASE}/common.pri )
QT +=  opengl 
