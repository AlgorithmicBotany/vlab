TEMPLATE = lib
CONFIG  += staticlib
SOURCES  = Message.cpp MessageQueue.cpp MessagePipe.cpp
TARGET   = message

MY_BASE  = ../..
MY_LIBS  = misc
include( $${MY_BASE}/common.pri )
