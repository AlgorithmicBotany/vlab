# this file should be included by all .pro files in vlab

# turn warnings on/off
# --------------------------------------------------------------------------------
  CONFIG += warn_on
# CONFIG += warn_off

# whether to do debugging
# --------------------------------------------------------------------------------
  CONFIG -= debug
  CONFIG += release

QMAKE_CXXFLAGS_RELEASE -= -Os
QMAKE_CXXFLAGS_RELEASE += -O3

# use ccache or no
# --------------------------------------------------------------------------------
#  QMAKE_CXX = ccache $$QMAKE_CXX
#  QMAKE_CC  = ccache $$QMAKE_CC

# where do temporaries go
# --------------------------------------------------------------------------------
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
  UI_DIR = .uic

DEFINES += QT_CLEAN_NAMESPACE

# platform specific defines
# --------------------------------------------------------------------------------
macx:{
  DEFINES += VLAB_MACX
# Uncomment this line to create a bundle with plugins
  MAKE_BUNDLE = true
  #CONFIG += x86 x86_64
  #QMAKE_MAC_SDK=/Developer/SDKs/MacOSX10.6.sdk 
}
unix:!macx {
    DEFINES += VLAB_LINUX
    LIBS += -lGLU  
}

# where do libraries & executables go
equals( TEMPLATE, lib ) {
  DESTDIR = $${MY_BASE}/.libraries
}
macx {
  equals( TEMPLATE, app ) {
    DESTDIR = $${MY_BASE}/.binaries
  }
}
unix:!macx{
  equals(TEMPLATE,app){
    DESTDIR = $${MY_BASE}/.binaries/bin/
    HELPDIR = $${MY_BASE}/.binaries/docs/
  }
}

# setup variables based on MY_LIBS
# --------------------------------------------------------------------------------

MY_NAME = RA
include ( ./expand_my_name.pri )

MY_NAME = message
include ( ./expand_my_name.pri )

MY_NAME = vlabd
include ( ./expand_my_name.pri )

MY_NAME = misc
include ( ./expand_my_name.pri )

MY_NAME = comm
include ( ./expand_my_name.pri )

MY_NAME = platform
include ( ./expand_my_name.pri )

MY_NAME = image
include ( ./expand_my_name.pri )

MY_NAME = shapes
include ( ./expand_my_name.pri )

MY_NAME = rand48
include ( ./expand_my_name.pri )

MY_NAME = rayimage
include ( ./expand_my_name.pri )

MY_NAME = rayobj
include ( ./expand_my_name.pri )

MY_NAME = raysurf
include ( ./expand_my_name.pri )

MY_NAME = raylight
include ( ./expand_my_name.pri )

MY_NAME = raycommon
include ( ./expand_my_name.pri )

MY_NAME = raytext
include ( ./expand_my_name.pri )

MY_NAME = lshade
include ( ./expand_my_name.pri )

MY_NAME = directoryWatcher
include ( ./expand_my_name.pri )


!isEmpty(MY_LIBS) {
	error( "Unknown libraries in MY_LIBS: $$MY_LIBS" )
}
