TEMPLATE = app
message( config = $$CONFIG)
SOURCES = main.cpp
TARGET = updatebin
QT += widgets
MY_BASE = ..
MY_LIBS = platform misc
include( $${MY_BASE}/common.pri )
CONFIG += qt
CONFIG -= gui
CONFIG -= app_bundle
