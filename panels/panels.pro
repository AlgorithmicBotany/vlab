TEMPLATE = app
CONFIG += qt \
opengl
CONFIG += debug
SOURCES = main.cpp \
          panel.cpp \
          glwidget.cpp \
          items.cpp \
          paneledit.cpp \
          itemdialogs.cpp \
          resources.cpp
 HEADERS = panel.h \
           glwidget.h \
           paneledit.h \
           itemdialogs.h \
           items.h resources.h
TARGET = panel
MY_BASE = ..
MY_LIBS = misc
include( $${MY_BASE}/common.pri )
# The following line was inserted by qt3to4
QT += opengl 
#    qt3support
macx:{
# 
#HEADERS +=		cocoabridge.h 
#OBJECTIVE_SOURCES +=	cocoabridge.mm 

LIBS +=		-framework AppKit -framework Carbon
LOCALDIR = $$DESTDIR/panel.app/Contents/Resources/English.lproj
HELPDIR = $$DESTDIR/panel.app/Contents/Resources/Panel.help/Contents/Resources/English.lproj
           
           info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/medit.app/Contents/

  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/panel.app/Contents/ &&  mkdir -p $$HELPDIR && cp -rf ../docs/VLABToolsManual.pdf $$HELPDIR  && cp -rf ../docs/Quick_Help/PanelQuickHelp.html $$HELPDIR   && cp -rf ../docs/files/*.* $$HELPDIR
  doc_info_list.target = doc_info_list

  QMAKE_EXTRA_TARGETS +=  info_list doc_info_list

  PRE_TARGETDEPS +=  info_list doc_info_list


}

RESOURCES += panels.qrc ../libs/misc/about.qrc
