TEMPLATE = app
#CONFIG  += console
SOURCES  = cccp.c  cexp.c  prefix.c  version.c
HEADERS = config.h  gansidecl.h  pcp.h
TARGET = vlabcpp

MY_BASE  = ..
MY_LIBS  =
include( $${MY_BASE}/common.pri )
QT += widgets
