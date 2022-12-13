TEMPLATE = app
CONFIG  -= qt
LIBS     = -lreadline
SOURCES  = raserver.cpp confirm_login.cpp edit_passwords.cpp  permissions.cpp
TARGET   = raserver

MY_BASE  = ..
#MY_LIBS  = platform message RA misc
MY_LIBS  = message RA misc
include( $${MY_BASE}/common.pri )
unix:!macx {
   LIBS += -ltermcap -lcrypt
}
#QT = core

