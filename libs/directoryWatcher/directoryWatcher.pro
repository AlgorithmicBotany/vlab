TEMPLATE = lib
CONFIG  += staticlib
TARGET   = directoryWatcher
SOURCES  = directorywatcher.cpp
HEADERS += directorywatcher.h

macx:{
SOURCES  += qvlabfilesystemwatcher.cpp
HEADERS += qvlabfilesystemwatcher.h 
}

QT  += widgets opengl core 

#QT = core

MY_BASE  = ../..
MY_LIBS = message misc
include( $${MY_BASE}/common.pri )

