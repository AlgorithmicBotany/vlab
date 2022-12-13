TEMPLATE = app
LANGUAGE = C++
CONFIG += qt
HEADERS += QTbrowser.h \
    QTGLbrowser.h \
    FindDialog.h \
    CustomizeDialog.h \
    ColorButton.h \
    FinderWidget.h \
    newbrowser.h \
    Export.h \
    Import.h \
    vlab_help.h \
    tree.h \
    SettingsStorage.h \
    readPixmap.h \
    openNode.h \
    main.h \
    log.h \
    lists_same.h \
    layout.h \
    graphics.h \
    font.h \
    eventHandle.h \
    environment.h \
    dragDrop.h \
    comm.h \
    buildTree.h \
    BrowserSettings.h \
    archive.h \
    FileConflict.h \
    TransferObject.h \ 
    resources.h qtsupport.h
SOURCES += main.cpp \
    graphics.cpp \
    font.cpp \
    tree.cpp \
    comm.cpp \
    eventHandle.cpp \
    openNode.cpp \
    layout.cpp \
    buildTree.cpp \
    dragDrop.cpp \
    FindDialog.cpp \
    log.cpp \
    QTbrowser.cpp \
    QTGLbrowser.cpp \
    lists_same.cpp \
    archive.cpp \
    vlab_help.cpp \
    CustomizeDialog.cpp \
    BrowserSettings.cpp \
    SettingsStorage.cpp \
    ColorButton.cpp \
    Node.cpp \
    readPixmap.cpp \
    FinderWidget.cpp \
    newbrowser.cpp \
    Export.cpp \
    Import.cpp \
    FileConflict.cpp \
    TransferObject.cpp \
    resources.cpp 
macx: { 
    SOURCES += environment.cpp 
    HEADERS += browserapp.h
    SOURCES += browserapp.cpp
    ICON = browser.icns
    LIBS += -framework \
        Carbon
}
QMAKE_COMPILER_DEFINES += __APPLE__ __GNUC__

FORMS =  FindDialog.ui ColorButton.ui  newbrowser.ui \
    CustomizeDialog.ui \
    Export.ui \
    Import.ui \
    FileConflict.ui\
    
FORMS += FixOofsDialog.ui
SOURCES += FixOofsDialog.cpp
HEADERS += FixOofsDialog.h
TARGET = browser

MY_BASE = ..
MY_LIBS = misc \
    RA \
    message \
    vlabd \
    platform
    include( $${MY_BASE}/common.pri )


       
!isEmpty(MAKE_BUNDLE) {
# QMAKE_INFO_PLIST = Info.plist
  DEFINES += USE_PLUGINS
  LOCALDIR = $$DESTDIR/browser.app/Contents/Resources/English.lproj
  HELPDIR =  $$DESTDIR/browser.app/Contents/Resources/Browser.help/Contents/Resources/English.lproj

  copyhelp.target = copyhelp
  copyhelp.commands = mkdir -p $$HELPDIR && cp -r '../docs/*' $$HELPDIR && cp -r '../docs/Quick_Help/BrowserQuickHelp.html' $$HELPDIR&& cp -rf ../docs/files/*.* $$HELPDIR
  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/browser.app/Contents/

  doc_info_list.target = doc_info_list
#  doc_info_list.commands = ( cp doc_Info.plist $$HELPDIR/../../Info.plist && cp InfoPlist.strings $$LOCALDIR/)

  QMAKE_EXTRA_TARGETS += copyhelp info_list doc_info_list

  PRE_TARGETDEPS += copyhelp info_list doc_info_list

} 
macx:{
# 
HEADERS +=		CocoaBridge.h 
OBJECTIVE_SOURCES +=	CocoaBridge.mm

 LIBS +=		-framework AppKit
}

unix:!macx{
LOCALDIR = $$DESTDIR/../
HELPDIR = $$HELPDIR/
mkdir.commands = ($$QMAKE_MKDIR $$HELPDIR)
mkdir.target = $$HELPDIR
copyhelp.target = copyhelp
copyhelp.commands = ( mkdir -p $$HELPDIR && cp -rf '../docs/*' $$HELPDIR )
QMAKE_EXTRA_TARGETS += copyhelp 
PRE_TARGETDEPS += copyhelp 
}

RESOURCES = browser.qrc ../libs/misc/about.qrc
CONFIG += debug
QT += widgets opengl 

