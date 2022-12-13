TEMPLATE = lib
CONFIG += staticlib
QT += widgets
TARGET = misc
SOURCES = buttons.cpp \
    xmemory.cpp \
    debug.cpp \
    utilities.cpp \
    dirList.cpp \
    Mem_IO.cpp \
    xstring.cpp \
    dsprintf.cpp \
    icon.cpp \
    delete_recursive.cpp \
    parse_object_location.cpp \
    xutils.cpp \
    Mem.cpp \
    Archive.cpp \
    QTask_login.cpp \
    QTStringDialog.cpp \
    rgbToAbgr.cpp \
    scale.cpp \
    hash.cpp \
    globMatch.cpp \
    connect_to.cpp \
    about.cpp \
    version.cpp \
    sgiFormat.cpp \
    dos2unix.cpp \
    rgbFormat.cpp \
    lodepng.cpp \
    lodepng_util.cpp \
    qtFontUtils.cpp \
    resources.cpp
HEADERS = QTask_login.h \
    QTStringDialog.h \
    dos2unix.h \
    sgiFormat.h \
    lodepng.h \
    lodepng_util.cpp  about.h
RESOURCES = logo.qrc about.qrc
FORMS = UI_RALogin.ui about.ui
MY_BASE = ../..
MY_LIBS = 
include( $${MY_BASE}/common.pri )
