TEMPLATE = app
LANGUAGE = C
CONFIG += link_prl
SOURCES  = main.c graphics.c render.c version.c
TARGET   = raypaint
INCLUDEPATH += ../ ../libray ../libshade /usr/X11/include
QMAKE_LINK = $$QMAKE_LINK_C

macx: {LIBS += -lm  -lz -framework GLUT -framework OpenGL}
#macx: {LIBS += -lm  -framework GLUT -framework OpenGL}
else{
CONFIG += no_lflags_merge
LIBS += -lm -L/usr/X11/lib -lGL -lGLU -lglut  -Wl,--start-group  ../../.libraries/liblshade.a ../../.libraries/libraytext.a ../../.libraries/librayimage.a \
        ../../.libraries/libraycommon.a ../../.libraries/libraysurf.a \
        ../../.libraries/librayobj.a ../../.libraries/libraylight.a -Wl,--end-group
}
MY_BASE  = ../..
MY_LIBS  = raytext lshade rayimage raycommon raysurf  rayobj raylight 
include( $${MY_BASE}/common.pri )
#QT = core
