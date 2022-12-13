TEMPLATE = app
CONFIG	+= qt
SOURCES	 = splash.cpp
TARGET   = vlab-splash
QT += widgets
#macx:IMAGES  += Mac-logo/logo.png
#!macx:IMAGES += Linux-logo/logo.png


MY_BASE  = ..
MY_LIBS  = misc
include( $${MY_BASE}/common.pri )

macx:RESOURCES     = ressources-mac.qrc
!macx:RESOURCES     = ressources-linux.qrc

