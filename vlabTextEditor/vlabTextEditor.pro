TEMPLATE = app
CONFIG  += qt opengl
HEADERS       = highlighter.h mainwindow.h codeeditor.h 
SOURCES       = main.cpp highlighter.cpp codeeditor.cpp resources.cpp\
mainwindow.cpp 

TARGET = vlabTextEdit


#! [0]
RESOURCES     = application.qrc ../libs/misc/about.qrc
#! [0]


MY_BASE  = ..
MY_LIBS  = directoryWatcher misc
include( $${MY_BASE}/common.pri )

   QT +=  opengl 

macx: { 
 LIBS += -framework \
 Carbon
}
!isEmpty(MAKE_BUNDLE) {
 # QMAKE_INFO_PLIST = Info.plist

  LOCALDIR = $$DESTDIR/vlabTextEdit.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/vlabTextEdit.app/Contents/Resources/vlabTextEdit.help/Contents/Resources/English.lproj

  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/vlabTextEdit.app/Contents/

  copyhelp.target = copyhelp
  copyhelp.commands = ( mkdir -p $$HELPDIR && mkdir -p $$LOCALDIR  && cp -rf ../docs/VLABToolsManual.pdf $$HELPDIR   && cp -rf ../docs/files/*.* $$HELPDIR)
  QMAKE_EXTRA_TARGETS += copyhelp info_list

  PRE_TARGETDEPS += copyhelp info_list


}
