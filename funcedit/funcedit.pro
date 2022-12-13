TEMPLATE = app
CONFIG  += qt opengl
SOURCES  = ctrl.cpp colors.cpp file.cpp functask.cpp glutils.cpp gridtask.cpp \
	   gridview.cpp main.cpp model.cpp namedlg.cpp resources.cpp mainwindow.cpp Preferences.cpp sampleDlg.cpp
HEADERS  = ctrl.h namedlg.h mainwindow.h Preferences.h sampleDlg.h
TARGET   = funcedit
#IMAGES = Images/icon.png \
#         Images/icon

FORMS +=  Preferences.ui


macx: {
LIBS += -framework Carbon
HEADERS +=		../libs/misc/CocoaBridge.h 
OBJECTIVE_SOURCES +=	../libs/misc/CocoaBridge.mm

LIBS +=		-framework AppKit
}

MY_BASE  = ..
MY_LIBS  = misc RA message platform 
include( $${MY_BASE}/common.pri )

RESOURCES     = funcedit.qrc ../libs/misc/about.qrc


QT +=  opengl

!isEmpty(MAKE_BUNDLE) {
 # QMAKE_INFO_PLIST = Info.plist

  LOCALDIR = $$DESTDIR/funcedit.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/funcedit.app/Contents/Resources/FuncEdit.help/Contents/Resources/English.lproj

  copyhelp.target = copyhelp
  copyhelp.commands = mkdir -p $$HELPDIR && cp -rf ../docs/VLABToolsManual.pdf $$HELPDIR && cp -rf ../docs/Quick_Help/FunceditQuickHelp.html $$HELPDIR && cp -rf ../docs/files/*.html $$HELPDIR && cp -rf ../docs/files/*.pdf $$HELPDIR
  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/funcedit.app/Contents/
  doc_info_list.target = doc_info_list
  doc_info_list.commands = cp doc_Info.plist $$HELPDIR/../../Info.plist

  QMAKE_EXTRA_TARGETS += copyhelp info_list doc_info_list

  PRE_TARGETDEPS += copyhelp info_list doc_info_list
}

# the funcedit manual is copied from the 'docs' directory
# and is now part of the vlab tools manual
#unix:!macx{
#LOCALDIR = $$DESTDIR/../
#FuncEdit_HELPDIR = $$HELPDIR/funcedit/
#mkdir.commands = ($$QMAKE_MKDIR $$FuncEdit_HELPDIR)
#mkdir.target = $$FuncEdit_HELPDIR
#copyhelp.target = copyhelp
#copyhelp.commands = ( mkdir -p $$FuncEdit_HELPDIR && cp -rf 'FuncEditManual/FuncEditHelp/*' $$FuncEdit_HELPDIR  )
#QMAKE_EXTRA_TARGETS +=  copyhelp info_list doc_info_list
#PRE_TARGETDEPS +=   copyhelp info_list doc_info_list
#}
