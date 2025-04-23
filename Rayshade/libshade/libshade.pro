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

flex.name = flex ${QMAKE_FILE_IN}
flex.input = FLEXSOURCES
flex.output = .obj/lex.o
flex.commands = flex -l  -t lex.l > lex.c
flex.CONFIG += target_predeps
flex.variable_out = GENERATED_SOURCES
silent:flex.commands = @echo Lex ${QMAKE_FILE_IN} && $$flex.commands
QMAKE_EXTRA_COMPILERS += flex

BISONSOURCES = yacc.y
bison_decl.name = bison_decl
bison_decl.input = BISONSOURCES
bison_decl.variable_out = GENERATED_FILES
bison_decl.commands = \
    -$(DEL_FILE) y.tab.h y.tab.c yacc.h yacc.c$$escape_expand(\\n\\t) \  
    bison -y -d ${QMAKE_FILE_IN}$$escape_expand(\\n\\t) \
    $(COPY) y.tab.h yacc.h$$escape_expand(\\n\\t) \
    $(COPY) y.tab.c yacc.c
bison_decl.output = yacc.h
bison_decl.dependency_type = TYPE_C
QMAKE_EXTRA_COMPILERS += bison_decl

bison_impl.name = bisonsource
bison_impl.input = BISONSOURCES
bison_impl.variable_out = GENERATED_SOURCES
bison_impl.commands = $$escape_expand(\\n)
bison_impl.depends = yacc.h
bison_impl.output = yacc.c
QMAKE_EXTRA_COMPILERS += bison_impl

MY_LIBS =  rayimage raycommon raysurf raytext rayobj raylight misc
#QT = core
macx {
  DEFINES += MACX_OPENGL_HEADERS
  LIBS += -framework Carbon
}

MY_BASE  = ../..
include( $${MY_BASE}/common.pri )
QMAKE_CFLAGS -= Wall

