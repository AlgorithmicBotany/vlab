TEMPLATE = app
QT += widgets

HEADERS +=  mainwindow.h rubberband.h Preferences.h resources.h SaveAs.h
SOURCES +=  mainwindow.cpp  main.cpp rubberband.cpp Preferences.cpp resources.cpp SaveAs.cpp
FORMS +=   Preferences.ui SaveAs.ui


RESOURCES = snapicon.qrc  ../libs/misc/about.qrc
macx: {
  LIBS += -framework Carbon
}

QT += opengl # qt3support 

MY_BASE  = ..
MY_LIBS  = misc platform
#QMAKE_INFO_PLIST = Info.plist

include( $${MY_BASE}/common.pri )
#The following line was inserted by qt3to4
!isEmpty(MAKE_BUNDLE) {
 # QMAKE_INFO_PLIST = Info.plist

  LOCALDIR = $$DESTDIR/snapicon.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/snapicon.app/Contents/Resources/Snapicon.help/Contents/Resources/English.lproj

  copyhelp.target = copyhelp
  copyhelp.commands = ( mkdir -p $$HELPDIR && mkdir -p $$LOCALDIR && cp -rf ../docs/VLABFramework.pdf $$HELPDIR && cp -rf ../docs/Quick_Help/SnapiconQuickHelp.html $$HELPDIR  && cp -rf ../docs/files/*.* $$HELPDIR)
  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/snapicon.app/Contents/
  QMAKE_EXTRA_TARGETS += copyhelp info_list 

  PRE_TARGETDEPS += copyhelp info_list 

}
