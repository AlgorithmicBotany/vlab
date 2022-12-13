TEMPLATE = lib
CONFIG  += staticlib
SOURCES  = platform.cpp
TARGET   = platform
QT -= gui

MY_BASE  = ../..
MY_LIBS  = 
include( $${MY_BASE}/common.pri )
