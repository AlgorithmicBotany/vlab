TEMPLATE = app
CONFIG  += qt opengl
SOURCES  = main.cpp mw.cpp mweditor.cpp mwviewer.cpp mwdialogs.cpp \
	   colourpick.cpp colour.cpp resources.cpp
HEADERS  = mweditor.h mwviewer.h mw.h mwdialogs.h colourpick.h
TARGET   = medit

macx: {
  LIBS += -framework Carbon
  SOURCES += mainwindow.cpp
  HEADERS += mainwindow.h
  }

  RESOURCES = medit.qrc  ../libs/misc/about.qrc



MY_BASE  = ..
MY_LIBS  = misc
include( $${MY_BASE}/common.pri )
#CONFIG += debug

QT +=  opengl
macx:{
# 
HEADERS +=		cocoabridge.h 
OBJECTIVE_SOURCES +=	cocoabridge.mm

 LIBS +=		-framework AppKit
}

!isEmpty(MAKE_BUNDLE) {
 # QMAKE_INFO_PLIST = Info.plist

  LOCALDIR = $$DESTDIR/medit.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/medit.app/Contents/Resources/Medit.help/Contents/Resources/English.lproj

  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/medit.app/Contents/

  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/medit.app/Contents/ &&  mkdir -p $$HELPDIR && cp -rf ../docs/VLABToolsManual.pdf $$HELPDIR  && cp -rf ../docs/Quick_Help/MeditQuickHelp.html $$HELPDIR   && cp -rf ../docs/files/*.* $$HELPDIR
  doc_info_list.target = doc_info_list

  QMAKE_EXTRA_TARGETS +=  info_list doc_info_list

  PRE_TARGETDEPS +=  info_list doc_info_list

}

