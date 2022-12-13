TEMPLATE = lib
CONFIG   += staticlib
TARGET   = vlabd
SOURCES  = libvlabd.cpp

MY_BASE  = ../..
MY_LIBS  = message misc
include( $${MY_BASE}/common.pri )

