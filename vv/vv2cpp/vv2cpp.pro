TEMPLATE = app
CONFIG += qt yacc_no_name_mangle
SOURCES = main.cpp
TARGET = vv2cpp

QMAKE_LEX = flex
LEXSOURCES = vvp.l

QMAKE_CXXFLAGS += -Wno-deprecated-register

BISONSOURCES = vvp.y

bison_decl.name = bison_decl
bison_decl.input = BISONSOURCES
bison_decl.variable_out = GENERATED_FILES
bison_decl.commands = \
-$(DEL_FILE) vvp.tab.h vvp.tab.c yacc.h yacc.cpp$$escape_expand(\\n\\t) \  
bison -d ${QMAKE_FILE_IN}$$escape_expand(\\n\\t) \
$(COPY) vvp.tab.h yacc.h$$escape_expand(\\n\\t) \
$(COPY) vvp.tab.c yacc.cpp
bison_decl.output = yacc.h
bison_decl.dependency_type = TYPE_C
QMAKE_EXTRA_COMPILERS += bison_decl

bison_impl.name = bisonsource
bison_impl.input = BISONSOURCES
bison_impl.variable_out = GENERATED_SOURCES
bison_impl.commands = $$escape_expand(\\n)
bison_impl.depends = yacc.h
bison_impl.output = yacc.cpp
QMAKE_EXTRA_COMPILERS += bison_impl

XXDSOURCES = ../generation/vvwrappers.hpp
xxd.output = ../generation/vvwrappers.xxd.h
xxd.commands = xxd -i ${QMAKE_FILE_IN} > ${QMAKE_FILE_OUT}
xxd.depends = $$XXDSOURCES
xxd.input = XXDSOURCES
xxd.variable_out = HEADERS
QMAKE_EXTRA_COMPILERS += xxd

MY_BASE  = ../..
MY_LIBS  =
include( $${MY_BASE}/common.pri )
