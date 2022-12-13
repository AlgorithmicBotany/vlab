TEMPLATE = lib
LANGUAGE = C
CONFIG  += staticlib yacc_no_name_mangle
CONFIG -= qt
TARGET   = lshade
SOURCES  = setup.c viewing.c shade.c picture.c  builtin.c symtab.c misc.c lightdef.c objdef.c options.c \
		stats.c surfdef.c yacc.c lex.c 
HEADERS = ../config.h  funcdefs.h \
		../patchlevel.h rayshade.h 

INCLUDEPATH += ./ ../ ../libray/ ../../cpfg/ ../../misc/libs/
QMAKE_LINK = $$QMAKE_LINK_C

flex.name = Flex ${QMAKE_FILE_IN}
flex.input = FLEXSOURCES
flex.output = .obj/lex.o
flex.commands = flex -l  -t lex.l > lex.c
flex.CONFIG += target_predeps
#flex.variable_out = GENERATED_SOURCES
silent:flex.commands = @echo Lex ${QMAKE_FILE_IN} && $$flex.commands
QMAKE_EXTRA_COMPILERS += flex

FLEXSOURCES = lex.l

#bison.name = Bison ${QMAKE_FILE_IN}
bison.input = BISONSOURCES
bison.output = ./y.tab.h
bison.commands = bison -y -d yacc.y 
#bison -y -d -o ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.parser.cpp ${QMAKE_FILE_IN}
bison.CONFIG += target_predeps
#bison.variable_out = GENERATED_SOURCES
silent:bison.commands = @echo Bison ${QMAKE_FILE_IN} && $$bison.commands

QMAKE_EXTRA_TARGETS += bison flex
PRE_TARGETDEPS += flex bison
BISONSOURCES = yacc.y

QMAKE_EXTRA_COMPILERS += bison

MY_LIBS =  rayimage raycommon raysurf raytext rayobj raylight misc
#QT = core
macx {
  DEFINES += MACX_OPENGL_HEADERS
  LIBS += -framework Carbon
}

MY_BASE  = ../..
include( $${MY_BASE}/common.pri )
QMAKE_CFLAGS -= Wall

#mv -f y.tab.c yacc.c
#gcc -g -arch x86_64  -I.. -I../libray -I/usr/X11/include -O3   -c -o yacc.o yacc.c
#flex -l  -t lex.l > lex.c
#gcc -g -arch x86_64  -I.. -I../libray -I/usr/X11/include -O3   -c -o lex.o lex.c
