TEMPLATE = lib
CONFIG  += staticlib
CONFIG -= qt
TARGET   = rayobj
SOURCES  = blob.c bounds.c box.c cone.c csg.c cylinder.c disc.c grid.c \
	 hf.c instance.c list.c intersect.c geom.c plane.c poly.c \
	 roots.c sphere.c torus.c triangle.c
HEADERS = 
INCLUDEPATH += ../ ../.. /usr/X11/include

QMAKE_LINK = $$QMAKE_LINK_C
#QT = core
macx {
  DEFINES += MACX_OPENGL_HEADERS
  LIBS += -framework Carbon
}

MY_BASE  = ../../..
include( $${MY_BASE}/common.pri )
