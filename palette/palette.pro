TEMPLATE = app
CONFIG  += qt opengl
SOURCES  = main.cpp glcolourmap.cpp palette.cpp colourpick.cpp colour.cpp resources.cpp
HEADERS  = glcolourmap.h palette.h  colourpick.h
TARGET   = palette

macx: {
  LIBS += -framework Carbon
  SOURCES += mainwindow.cpp
  HEADERS += mainwindow.h
}

MY_BASE  = ..
MY_LIBS  = misc
include( $${MY_BASE}/common.pri )

RESOURCES = palette.qrc  ../libs/misc/about.qrc

QT +=  opengl widgets 

!isEmpty(MAKE_BUNDLE) {
 # QMAKE_INFO_PLIST = Info.plist

  LOCALDIR = $$DESTDIR/palette.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/palette.app/Contents/Resources/Palette.help/Contents/Resources/English.lproj

  copyhelp.target = copyhelp
  copyhelp.commands = mkdir -p $$HELPDIR && cp -Rf '../docs/Quick_Help/PaletteQuickHelp.html' $$HELPDIR && cp -rf ../docs/VLABToolsManual.pdf $$HELPDIR && cp -rf ../docs/files/*.* $$HELPDIR
  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/palette.app/Contents/
  doc_info_list.target = doc_info_list
  doc_info_list.commands = cp doc_Info.plist $$HELPDIR/../../Info.plist 

  QMAKE_EXTRA_TARGETS += copyhelp info_list doc_info_list

  PRE_TARGETDEPS += copyhelp info_list doc_info_list

}
