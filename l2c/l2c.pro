TEMPLATE = app
CONFIG  += qt yacc_no_name_mangle
SOURCES  = contextdata.cpp main.cpp module.cpp production.cpp
DEFINES  = LINUX
TARGET = l2c
INCLUDEPATH += ../lpfg/include

#copyhelp.target = copyhelp
#copyhelp.commands = cp -f l2c.l l.l

#QMAKE_EXTRA_TARGETS += copyhelp
#PRE_TARGETDEPS += copyhelp

QMAKE_LEX = flex
LEXSOURCES = l2c.l
QMAKE_LEXFLAGS += -Pl --prefix=l2c
#QMAKE_LEXSOURCES = lex.l2c.c

QMAKE_CXXFLAGS += -Wno-deprecated-register
YACCSOURCES = l2c.y
QMAKE_YACCFLAGS += -p l2c -o l2c.tab.c


MY_BASE  = ..
MY_LIBS  =
include( $${MY_BASE}/common.pri )
