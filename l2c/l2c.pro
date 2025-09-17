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

# yacc, qmake, and Xcode command line tools are incompatible, so use bison manually...
#macx: {
#  YACCSOURCES = l2c.y
#  QMAKE_YACCFLAGS += -p l2c -o l2c.tab.c
#}
#unix:!macx{
  BISONSOURCES = l2c.y

  bison_decl.name = bison_decl
  bison_decl.input = BISONSOURCES
  bison_decl.variable_out = GENERATED_FILES
  bison_decl.commands = \
    -$(DEL_FILE) l2c.tab.h l2c.tab.c l2c_yacc.h l2c_yacc.cpp$$escape_expand(\\n\\t) \  
    bison -d -p l2c -b l2c ${QMAKE_FILE_IN}$$escape_expand(\\n\\t) \
    $(COPY) l2c.tab.h l2c_yacc.h$$escape_expand(\\n\\t) \
    $(COPY) l2c.tab.c l2c_yacc.cpp
  bison_decl.output = l2c_yacc.h
  bison_decl.dependency_type = TYPE_C
  QMAKE_EXTRA_COMPILERS += bison_decl

  bison_impl.name = bisonsource
  bison_impl.input = BISONSOURCES
  bison_impl.variable_out = GENERATED_SOURCES
  bison_impl.commands = $$escape_expand(\\n)
  bison_impl.depends = l2c_yacc.h
  bison_impl.output = l2c_yacc.cpp 
  QMAKE_EXTRA_COMPILERS += bison_impl
# }

MY_BASE  = ..
MY_LIBS  =
include( $${MY_BASE}/common.pri )
