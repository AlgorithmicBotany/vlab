TEMPLATE = app
CONFIG  += qt
SOURCES  = vlabd.cpp getObject.cpp queue.cpp commutil.cpp socket.cpp list.cpp
TARGET   = vlabd

MY_BASE  = ..
MY_LIBS  = misc vlabd
include( $${MY_BASE}/common.pri )
QT += widgets opengl 
