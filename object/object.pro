TEMPLATE = app
CONFIG  += qt opengl
SOURCES  = object.cpp QTGLObject.cpp \
    auxMenuItem.cpp saveChanges.cpp \
    util.cpp labutil.cpp resources.cpp \
    MessageBox.cpp objectMessageBox.cpp \
    TransferObject.cpp newVersion.cpp directorywatcher.cpp ImportExport.cpp FileConflict.cpp Preferences.cpp

HEADERS  = QTGLObject.h PostExec.h Loader.h MessageBox.h TransferObject.h newVersion.h directorywatcher.h objectMessageBox.h util.h saveChanges.h object.h labutil.h auxMenuItem.h ImportExport.h FileConflict.h Preferences.h

FORMS +=  ImportExport.ui  \
          FileConflict.ui \
          Preferences.ui 

TARGET   = object

macx: {
  LIBS += -framework Carbon

  # to speed-up testing, make install will copy the object.app bundle to the Distribution directory
  # this is much faster than calling ./compile-all.sh from the root vlab directory
  # however, the vlab-x.x.x, version number has to be updated manually in the next line
  object_quick_install.path = ../Distribution/vlab-5.0.0/browser.app/Contents/System/
  object_quick_install.files = ../.binaries/object.app
  INSTALLS += object_quick_install
}
SOURCES += mainwindow.cpp
HEADERS += mainwindow.h


MY_BASE  = ..
MY_LIBS  = misc RA message vlabd platform
include( $${MY_BASE}/common.pri )
RESOURCES = object.qrc ../libs/misc/about.qrc

QT += opengl 

!isEmpty(MAKE_BUNDLE) {
# QMAKE_INFO_PLIST = Info.plist

  LOCALDIR = $$DESTDIR/object.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/object.app/Contents/Resources/Object.help/Contents/Resources/English.lproj

  copyhelp.target = copyhelp
  copyhelp.commands = mkdir -p $$HELPDIR && cp -f ../docs/VLABFramework.pdf $$HELPDIR && cp -rf ../docs/files/*.* $$HELPDIR # && cp -f ../docs/Quick_Help/ObjectQuickHelp.html $$HELPDIR   
  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/object.app/Contents/
  doc_info_list.target = doc_info_list

  QMAKE_EXTRA_TARGETS += copyhelp info_list doc_info_list

  PRE_TARGETDEPS += copyhelp info_list doc_info_list
}

