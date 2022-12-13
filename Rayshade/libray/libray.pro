TEMPLATE =  subdirs lib 
LANGUAGE = C
INCLUDEPATH += /usr/local/include
MY_BASE = ..
SUBDIRS = libimage libobj libsurf libcommon liblight libtext
TARGET = ray
MY_BASE  = ../..
include( $${MY_BASE}/common.pri )

