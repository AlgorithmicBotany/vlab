TEMPLATE = app
TARGET = timeline
CONFIG += qt \
    opengl \
    warn_on \
    release


SOURCES += \
        main.cpp \
        mainwindow.cpp \
        timeline.cpp \
        EventButton.cpp \
        event.cpp \
        addpointdialog.cpp \
        deleteconfirm.cpp \
        editpointdialog.cpp \
        helpdialog.cpp \
        file.cpp \
        colors.cpp \
        Preferences.cpp \
        resources.cpp
        
HEADERS += \
        mainwindow.h \
        timeline.h \
        EventButton.h \
        event.h \
        addpointdialog.h \
        deleteconfirm.h \
        editpointdialog.h \
        helpdialog.h \
        file.h \
        colors.h \
        Preferences.h \
        resources.h

FORMS +=  mainwindow.ui Preferences.ui

        # The following define makes your compiler emit warnings if you use
        # any feature of Qt which has been marked as deprecated (the exact warnings
        # depend on your compiler). Please consult the documentation of the
        # deprecated API in order to know how to port your code away from it.
        DEFINES += QT_DEPRECATED_WARNINGS
        
        # You can also make your code fail to compile if you use deprecated APIs.
        # In order to do so, uncomment the following line.
        # You can also select to disable deprecated APIs only up to a certain version of Qt.
        #DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


MY_BASE = ..
MY_LIBS = misc directoryWatcher platform
RESOURCES += resources.qrc  ../libs/misc/about.qrc

include( $${MY_BASE}/common.pri )

 macx: { 
 LIBS += -framework \
 Carbon

 LOCALDIR = $$DESTDIR/timeline.app/Contents/Resources/English.lproj
 HELPDIR = $$DESTDIR/timeline.app/Contents/Resources/timeline.help/Contents/Resources/English.lproj
 info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/timeline.app/Contents/

  copyhelp.target = copyhelp
  copyhelp.commands = ( mkdir -p $$HELPDIR && mkdir -p $$LOCALDIR  && cp -rf ../docs/VLABToolsManual.pdf $$HELPDIR && cp -rf ../docs/Quick_Help/TimelineQuickHelp.html $$HELPDIR  && cp -rf ../docs/files/*.html $$HELPDIR)
 QMAKE_EXTRA_TARGETS += copyhelp info_list

  PRE_TARGETDEPS += copyhelp info_list

}


QT       += core gui opengl widgets

