TEMPLATE = app
CONFIG  += qt opengl
SOURCES  = main.cpp ctrl.cpp model.cpp view.cpp viewtask.cpp perspview.cpp glutils.cpp config.cpp resources.cpp mainwindow.cpp
HEADERS  = ctrl.h view.h perspview.h mainwindow.h
TARGET   = bezieredit

macx: {
  LIBS += -framework Carbon
}

RESOURCES = bezieredit.qrc ../libs/misc/about.qrc

MY_BASE  = ..
MY_LIBS  = misc
include( $${MY_BASE}/common.pri )
QT +=  opengl widgets 

!isEmpty(MAKE_BUNDLE) {
 # QMAKE_INFO_PLIST = Info.plist

  LOCALDIR = $$DESTDIR/bezieredit.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/bezieredit.app/Contents/Resources/BezierEdit.help/Contents/Resources/English.lproj

  copyhelp.target = copyhelp
  copyhelp.commands = mkdir -p $$HELPDIR && cp -rf ../docs/VLABToolsManual.pdf $$HELPDIR && cp -rf ../docs/Quick_Help/BeziereditQuickHelp.html $$HELPDIR  && cp -rf ../docs/files/*.html $$HELPDIR
  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/bezieredit.app/Contents/

  QMAKE_EXTRA_TARGETS += copyhelp info_list

  PRE_TARGETDEPS += copyhelp info_list
}
