TEMPLATE = app
CONFIG += console release
QT += opengl
HEADERS += combinedpk.h \
           comm_lib.h \
           gldisplay.h \
           globjwin.h \
           korobov.h \
           matrix.h \
           mc.h \
           MRGrand.h \
           qmc.h \
           quasiMC.h \
           query.h \
           randqmc.h \
           randquasimc.h \
           ray.h \
           scene3d.h \
           sky.h \
           sobol.h \
           soboldata.h \
           surface.h
SOURCES += args.c \
           combinedpk.c \
           gldisplay.cpp \
           globjwin.cpp \
           korobov.c \
           main.cpp \
           matrix.c \
           mc.c \
           message.c \
           MRGrand.c \
           qmc.c \
           quasiMC.c \
           query.c \
           randqmc.c \
           randquasimc.c \
           scene3d.c \
           sky.c \
           sobol.c \
           statsqmc.c \
           surface.c
TARGET   = QuasiMC
VPATH += ../../libs/comm
INCLUDEPATH += ../../libs

MY_BASE  = ../..
MY_LIBS  = comm
include( $${MY_BASE}/common.pri )

# Add copying of resources for mac
!isEmpty(MAKE_BUNDLE) {
  LOCALDIR = $$DESTDIR/QuasiMC.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/QuasiMC.app/Contents/Resources/QuasiMC.help/Contents/Resources/English.lproj

  copyhelp.target = copyhelp
  copyhelp.commands = ( mkdir -p $$HELPDIR && cp -f '../../docs/EnviroManual.pdf' $$HELPDIR )

  QMAKE_EXTRA_TARGETS += copyhelp 
  PRE_TARGETDEPS += copyhelp 
}
