QT          += widgets opengl gui

HEADERS     = glwidget.h \
              editor.cpp \              
              set.h fset.h cset.h \
              item.h func.h contour.h  createitemdlg.h\
              gallery.h \
              config.h Preferences.h mainwindow.h
              
SOURCES     = glwidget.cpp \
              main.cpp \
              editor.cpp \
              set.cpp fset.cpp cset.cpp \
              item.cpp func.cpp contour.cpp createitemdlg.cpp \
              gallery.cpp \
              config.cpp mainwindow.cpp resources.cpp Preferences.cpp

FORMS +=  Preferences.ui

CONFIG += debug
# install
TARGET = gallery

RESOURCES += resources.qrc ../libs/misc/about.qrc
MY_BASE = ../../vlab
MY_LIBS = misc directoryWatcher platform
include( $${MY_BASE}/common.pri )
   
macx: {
LIBS += -framework Carbon
HEADERS +=		../libs/misc/CocoaBridge.h 
OBJECTIVE_SOURCES +=	../libs/misc/CocoaBridge.mm

LIBS +=		-framework AppKit


  LOCALDIR = $$DESTDIR/gallery.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/gallery.app/Contents/Resources/Gallery.help/Contents/Resources/English.lproj


  copyhelp.target = copyhelp
  copyhelp.commands = ( mkdir -p $$HELPDIR && mkdir -p $$LOCALDIR && cp -rf ../docs/VLABToolsManual.pdf $$HELPDIR &&  cp -rf ../docs/Quick_Help/GalleryQuickHelp.html $$HELPDIR  && cp -rf ../docs/files/*.* $$HELPDIR) 
     #  info_list.target = info_list
  QMAKE_EXTRA_TARGETS += copyhelp
   PRE_TARGETDEPS += copyhelp 
}
