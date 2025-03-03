TARGET   = cpfg
TEMPLATE = app
QT +=  opengl printsupport widgets core
CONFIG += qt opengl core

SOURCES  = background.c \
           blackbox.c \
           control.c \
           environment.c \
           general.c \
           generate.c \
           hash.c \
           interpret.c \
           irisGL.c \
           matrix.c \
           mesh.c \
           movements.c \
           object.c \
           outputgls.c \
           patches.c \
           postscript.c \
           quaternions.c \
           rayshade.c \
           rle.c \
           server.c \
           sphere.c \
           targa.c \
           test_malloc.c \
           textures.c \
           tsurfaces.c \
           turtle.c \
           utility.c \
           viewVol.c \
           animparam.c \
           writergb.c \
           lodepng.c \
           shaders.c \
           file.cpp \
           point.cpp \
           spline.cpp \
           splineXYZ.cpp \
           splinearr.cpp \
           splinefun.cpp \
           userfiles.cpp \
           curveXYZc.cpp \
           curveXYZ.cpp \
           curveXYZa.cpp \
           compactor.cpp \
           platformQt.cpp \
           menuQt.cpp \
           glcanvas.cpp \
           GLWindow.cpp \
           SaveAs.cpp \
           utils.cpp
           
HEADERS  = GLWindow.h \
           glcanvas.h \
           SaveAs.h
           
DEFINES  = LINUX DEBUG=0 CPFG_VERSION=6600 CPFG_ENVIRONMENT \
	   RESEARCH_VER CONTEXT_SENSITIVE_HOMO JIM _GLUT_WINDOWING DEPTH_CUEING DEPTHCUE

FORMS +=  SaveAs.ui 
 
RESOURCES = cpfg.qrc ../libs/misc/about.qrc

MY_BASE  = ..
MY_LIBS  = image comm misc directoryWatcher
include( $${MY_BASE}/common.pri )


macx: {
    #LEXSOURCES = lsys_input.l
    #YACCSOURCES = lsys_input.y

    DEFINES += MACX_OPENGL_HEADERS
    SOURCES += apple.cpp
    HEADERS += apple.h
    LIBS += -framework Carbon

    # to speed-up testing, make install will copy the cpfg.app bundle to the Distribution directory
    # this is much faster than calling ./compile-all.sh from the root vlab directory
    # however, the vlab-x.x.x, version number has to be updated manually in the next line
    cpfg_quick_install.path = ../Distribution/vlab-5.0.0/browser.app/Contents/Plug-ins/
    cpfg_quick_install.files = ../.binaries/cpfg.app
    INSTALLS += cpfg_quick_install
}

unix:!macx{
  SOURCES += apple.cpp
  HEADERS += apple.h
}

# qmake does not work automatically with lex/yacc/bison commands, so it needs to be done manually 
LEXSOURCES = lsys_input.l
BISONSOURCES = lsys_input.y

bison_decl.name = bison_decl
bison_decl.input = BISONSOURCES
bison_decl.variable_out = GENERATED_FILES
bison_decl.commands = \
    -$(DEL_FILE) lsys_input.tab.h lsys_input.tab.c lsys_input_yacc.h lsys_input_yacc.cpp$$escape_expand(\\n\\t) \  
    bison -d -p lsys_input -b lsys_input ${QMAKE_FILE_IN}$$escape_expand(\\n\\t) \
    $(COPY) lsys_input.tab.h lsys_input_yacc.h$$escape_expand(\\n\\t) \
    $(COPY) lsys_input.tab.c lsys_input_yacc.cpp
bison_decl.output = lsys_input_yacc.h
bison_decl.dependency_type = TYPE_C
QMAKE_EXTRA_COMPILERS += bison_decl

bison_impl.name = bisonsource
bison_impl.input = BISONSOURCES
bison_impl.variable_out = GENERATED_SOURCES
bison_impl.commands = $$escape_expand(\\n)
bison_impl.depends = lsys_input_yacc.h
bison_impl.output = lsys_input_yacc.cpp 
QMAKE_EXTRA_COMPILERS += bison_impl


QMAKE_CXXFLAGS += -Wno-deprecated-register
S

!isEmpty(MAKE_BUNDLE) {
QMAKE_INFO_PLIST = Info.plist

LOCALDIR = $$DESTDIR/cpfg.app/Contents/Resources/English.lproj
HELPDIR = $$DESTDIR/cpfg.app/Contents/Resources/CPFG.help/Contents/Resources/English.lproj
mkdir.commands = ($$QMAKE_MKDIR $$HELPDIR && $$QMAKE_MKDIR $$LOCALDIR)
mkdir.target = $$HELPDIR
copyhelp.target = copyhelp
copyhelp.commands = ( mkdir -p $$HELPDIR && cp -f ../docs/CPFGManual.pdf $$HELPDIR && cp -rf ../docs/EnviroManual.pdf $$HELPDIR && cp -rf ../docs/Quick_Help/CPFGQuickHelp.html $$HELPDIR  && cp -rf ../docs/files/*.* $$HELPDIR )
info_list.target = info_list
doc_info_list.target = doc_info_list

QMAKE_EXTRA_TARGETS +=  copyhelp info_list doc_info_list

PRE_TARGETDEPS +=   copyhelp info_list doc_info_list
}

# the cpfg manual is copied from the 'docs' directory
# so this is no longer needed
#unix:!macx{
#LOCALDIR = $$DESTDIR/../
#CPFG_HELPDIR = $$HELPDIR/cpfg/
#mkdir.commands = ($$QMAKE_MKDIR $$CPFG_HELPDIR)
#mkdir.target = $$CPFG_HELPDIR
#copyhelp.target = copyhelp
#copyhelp.commands = ( mkdir -p $$CPFG_HELPDIR && cp -rf '../docs/CPFGmanual.pdf' $$CPFG_HELPDIR )
#QMAKE_EXTRA_TARGETS +=  copyhelp info_list doc_info_list
#PRE_TARGETDEPS +=   copyhelp info_list doc_info_list
#}
