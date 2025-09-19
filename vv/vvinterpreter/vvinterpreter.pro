TEMPLATE = app
CONFIG +=
QT += opengl
TARGET = vvinterpreter
RESOURCES = gui.qrc ../../libs/misc/about.qrc
SOURCES = main.cpp dllinterface.cpp vvpapp.cpp vvpviewer.cpp icon.cpp
HEADERS = vvpapp.hpp vvpviewer.hpp 
INCLUDEPATH += ../vvlib ../
LIBS += -L../../.libraries -lvv

MY_BASE  = ../..
MY_LIBS  = misc 
include( $${MY_BASE}/common.pri )

macx {
  DEFINES += MACX_OPENGL_HEADERS
  LIBS += -framework Carbon

  include( $${MY_BASE}/config_lpfg.pri )
}

!isEmpty(MAKE_BUNDLE) {

  VV_DESTDIR = $$DESTDIR/vvinterpreter.app/Contents

  VV_HEADERS.files = ../vvlib/algebra ../vvlib/algorithms ../generation
  VV_HEADERS.path = Contents/Resources/include
  QMAKE_BUNDLE_DATA += VV_HEADERS

  VV_UTIL_HEADERS = ../vvlib/util/*.hpp 
  copy_util_headers.target = copy_util_headers
  copy_util_headers.depends = $$VV_UTIL_HEADERS
  copy_util_headers.commands = mkdir -p  $$VV_DESTDIR/Resources/include/util && cp -p $$VV_UTIL_HEADERS $$VV_DESTDIR/Resources/include/util

  VV_VIEW_HEADERS = ../vvlib/view/*.hpp 
  copy_view_headers.target = copy_view_headers
  copy_view_headers.depends = $$VV_VIEW_HEADERS
  copy_view_headers.commands = mkdir -p  $$VV_DESTDIR/Resources/include/view && cp -p $$VV_VIEW_HEADERS $$VV_DESTDIR/Resources/include/view

  VV_EXT_HEADERS = ../vvlib/external/*.h
  copy_ext_headers.target = copy_ext_headers
  copy_ext_headers.depends = $$VV_EXT_HEADERS
  copy_ext_headers.commands = mkdir -p  $$VV_DESTDIR/Resources/include/external && cp -p $$VV_EXT_HEADERS $$VV_DESTDIR/Resources/include/external

  VV_VVINTERPRETER_HEADERS = vvpviewer.hpp
  copy_vvinterpreter_headers.target = copy_vvinterpreter_headers
  copy_vvinterpreter_headers.depends = $$VV_VVINTERPRETER_HEADERS
  copy_vvinterpreter_headers.commands = mkdir -p  $$VV_DESTDIR/Resources/include/vvinterpreter && cp -p $$VV_VVINTERPRETER_HEADERS $$VV_DESTDIR/Resources/include/vvinterpreter

  VV_LIBS = $${MY_BASE}/.libraries/libvv*dylib
  copylibs.target = copylibs
  copylibs.depends = $$VV_LIBS
  copylibs.commands += mkdir -p $$VV_DESTDIR/Resources/libs && cp -p $$VV_LIBS $$VV_DESTDIR/Resources/libs

  message($$CONFIG)
  ARCH=""
  contains(CONFIG,x86) {
    ARCH+=" -arch i386 -stdlib=libc++"
  }
  contains(CONFIG,x86_64) {
    ARCH+=" -arch x86_64 -stdlib=libc++"
  }
  contains(CONFIG,arm64) {
    ARCH+=" -arch arm64"
  }
  copymak.target = copymak
  copymak.depends = program_macos.mk
  copymak.commands = ( mkdir -p $$VV_DESTDIR/Resources && sed -e \'s/-ARCH/$${ARCH}/\' program_macos.mk > $$VV_DESTDIR/Resources/program.mk )

  HELPDIR = $$VV_DESTDIR/Resources/VV.help/Contents/Resources/English.lproj
  copyhelp.target = copyhelp
  #copyhelp.commands = mkdir -p $$HELPDIR && cp -f ../../docs/VVManual.pdf $$HELPDIR && cp -f ../../docs/Quick_Help/VVQuickHelp.html $$HELPDIR && cp -rf ../../docs/files/*.* $$HELPDIR
  copyhelp.commands = mkdir -p $$HELPDIR && cp -rf ../../docs/files/*.* $$HELPDIR

  QMAKE_EXTRA_TARGETS += copy_view_headers copy_util_headers copy_ext_headers copy_vvinterpreter_headers copylibs copymak copyhelp
  PRE_TARGETDEPS += copy_view_headers copy_util_headers copy_ext_headers copy_vvinterpreter_headers copylibs copymak copyhelp

} else {

  VV_DESTDIR = ../../.binaries

  VV_HEADERS = ../vvlib/algebra ../vvlib/algorithms ../generation
  copy_headers.target = copy_headers
  copy_headers.depends = $$VV_HEADERS
  copy_headers.commands = mkdir -p $$VV_DESTDIR/include/vv && cp -r $$VV_HEADERS $$VV_DESTDIR/include/vv

  VV_UTIL_HEADERS = ../vvlib/util/*.hpp 
  copy_util_headers.target = copy_util_headers
  copy_util_headers.depends = $$VV_UTIL_HEADERS
  copy_util_headers.commands = mkdir -p $$VV_DESTDIR/include/vv/util && cp -p $$VV_UTIL_HEADERS $$VV_DESTDIR/include/vv/util

  VV_VIEW_HEADERS = ../vvlib/view/*.hpp 
  copy_view_headers.target = copy_view_headers
  copy_view_headers.depends = $$VV_VIEW_HEADERS
  copy_view_headers.commands = mkdir -p $$VV_DESTDIR/include/vv/view && cp -p $$VV_VIEW_HEADERS $$VV_DESTDIR/include/vv/view

  VV_EXT_HEADERS = ../vvlib/external/*.h
  copy_ext_headers.target = copy_ext_headers
  copy_ext_headers.depends = $$VV_EXT_HEADERS
  copy_ext_headers.commands = mkdir -p $$VV_DESTDIR/include/vv/external && cp -p $$VV_EXT_HEADERS $$VV_DESTDIR/include/vv/external

  VV_VVINTERPRETER_HEADERS = vvpviewer.hpp
  copy_vvinterpreter_headers.target = copy_vvinterpreter_headers
  copy_vvinterpreter_headers.depends = $$VV_VVINTERPRETER_HEADERS
  copy_vvinterpreter_headers.commands = mkdir -p $$VV_DESTDIR/include/vv/vvinterpreter && cp -p $$VV_VVINTERPRETER_HEADERS $$VV_DESTDIR/include/vv/vvinterpreter

  #HELPDIR = $$VV_DESTDIR/Resources/VV.help/Contents/Resources/English.lproj
  #copyhelp.target = copyhelp
  #copyhelp.commands = mkdir -p $$HELPDIR && cp -f ../../docs/VVManual.pdf $$HELPDIR && cp -f ../../docs/Quick_Help/VVQuickHelp.html $$HELPDIR && cp -rf ../../docs/files/*.* $$HELPDIR
  #copyhelp.commands = mkdir -p $$HELPDIR && cp -rf ../../docs/files/*.* $$HELPDIR

  QMAKE_EXTRA_TARGETS += copy_headers copy_view_headers copy_util_headers copy_ext_headers copy_vvinterpreter_headers copyhelp
  PRE_TARGETDEPS += copy_headers copy_view_headers copy_util_headers copy_ext_headers copy_vvinterpreter_headers copyhelp
}

