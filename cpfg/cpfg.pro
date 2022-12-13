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
LEXSOURCES = lsys_input.l
YACCSOURCES = lsys_input.y
FORMS +=  SaveAs.ui 

RESOURCES = cpfg.qrc ../libs/misc/about.qrc

MY_BASE  = ..
MY_LIBS  = image comm misc directoryWatcher
include( $${MY_BASE}/common.pri )


macx: {
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

QMAKE_CXXFLAGS += -Wno-deprecated-register




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
