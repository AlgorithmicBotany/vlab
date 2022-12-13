TEMPLATE = app
CONFIG  += qt opengl
SOURCES  = ctrl.cpp colors.cpp file.cpp functask.cpp glutils.cpp gridtask.cpp gridview.cpp \
	   main.cpp model.cpp namedlg.cpp resources.cpp mainwindow.cpp  Preferences.cpp sampleDlg.cpp
HEADERS  = ctrl.h namedlg.h mainwindow.h  Preferences.h sampleDlg.h
TARGET   = cuspy
RESOURCES = cuspy.qrc ../libs/misc/about.qrc
FORMS +=  Preferences.ui

MY_BASE  = ..
MY_LIBS  = misc platform
include( $${MY_BASE}/common.pri )
   
macx: {
LIBS += -framework Carbon
HEADERS +=		../libs/misc/CocoaBridge.h 
OBJECTIVE_SOURCES +=	../libs/misc/CocoaBridge.mm

 LIBS +=		-framework AppKit
}

QT +=  opengl 

!isEmpty(MAKE_BUNDLE) {
 # QMAKE_INFO_PLIST = Info.plist

  LOCALDIR = $$DESTDIR/cuspy.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/cuspy.app/Contents/Resources/Cuspy.help/Contents/Resources/English.lproj

  copyhelp.target = copyhelp
  copyhelp.commands = mkdir -p $$HELPDIR && cp '../docs/Quick_Help/CuspyQuickHelp.html' $$HELPDIR && cp -rf ../docs/files/*.* $$HELPDIR
  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/cuspy.app/Contents/
  doc_info_list.target = doc_info_list
  doc_info_list.commands = ( cp doc_Info.plist $$HELPDIR/../../Info.plist && cp -rf ../docs/VLABToolsManual.pdf $$HELPDIR )

  QMAKE_EXTRA_TARGETS += copyhelp info_list doc_info_list

  PRE_TARGETDEPS += copyhelp info_list doc_info_list
}

# the cuspy manual is copied from the 'docs' directory
# and is now part of the vlab tools manual
#unix:!macx{
#LOCALDIR = $$DESTDIR/../
#Cuspy_HELPDIR = $$HELPDIR/cuspy/
#mkdir.commands = ($$QMAKE_MKDIR $$Cuspy_HELPDIR)
#mkdir.target = $$Cuspy_HELPDIR
#copyhelp.target = copyhelp
#copyhelp.commands = ( mkdir -p $$Cuspy_HELPDIR && cp -rf 'CuspyManual/CuspyHelp/*' $$Cuspy_HELPDIR )
#QMAKE_EXTRA_TARGETS +=  copyhelp info_list doc_info_list
#PRE_TARGETDEPS +=   copyhelp info_list doc_info_list
#}
