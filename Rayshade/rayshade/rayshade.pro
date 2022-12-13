TEMPLATE = app
LANGUAGE = C
SOURCES  = main.c raytrace.c version.c
TARGET   = rayshade
CONFIG += create_prl link_prl
INCLUDEPATH += ../ ../libray ../libshade /usr/X11/include
QMAKE_LINK = $$QMAKE_LINK_C
macx: {
LIBS += -lm -lz 
}
else{
CONFIG += no_lflags_merge
LIBS += -lm -L/usr/X11/lib -Wl,--start-group ../../.libraries/librayimage.a ../../.libraries/librayobj.a ../../.libraries/libraysurf.a ../../.libraries/libraylight.a ../../.libraries/libraycommon.a ../../.libraries/libraytext.a ../../.libraries/liblshade.a -lm -Wl,--end-group
}
MY_BASE  = ../..
MY_LIBS  = rayimage raycommon raysurf raytext rayobj raylight lshade
include( $${MY_BASE}/common.pri )
#QT = core
