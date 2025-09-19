TEMPLATE = lib
#CONFIG  += staticlib
CONFIG  += shared
macx: {
QMAKE_SONAME_PREFIX = @executable_path/../Resources/libs/
}
QT += opengl
TARGET   = vv
SOURCES  = \
    ./util/contour.cpp \
    ./util/dir.cpp \
    ./util/function.cpp \
    ./util/materials.cpp \
    ./util/palette.cpp \
    ./util/parms.cpp \
    ./util/texture.cpp \
    ./view/orthoviewer.cpp \
    ./view/perspviewer.cpp \
    ./view/viewer.cpp \
    ./external/gl2ps.cpp

HEADERS = \
    ./algebra/abstractmesh.hpp \
    ./algebra/abstractvertex.hpp \
    ./algebra/opqueue.hpp \
    ./algebra/xmlmesh.hpp \
    ./algorithms/cloneset.hpp \
    ./algorithms/consistency.hpp \
    ./algorithms/insert.hpp \
    ./algorithms/render.hpp \
    ./algorithms/stellar.hpp \
    ./util/buffer.hpp \
    ./util/clamp.hpp \
    ./util/colour.hpp \
    ./util/contour.hpp \
    ./util/dir.hpp \
    ./util/forall.hpp \
    ./util/function.hpp \
    ./util/geometry.hpp \
    ./util/glerrorcheck.hpp \
    ./util/materials.hpp \
    ./util/matrix_impl.hpp \
    ./util/matrix.hpp \
    ./util/minmax.hpp \
    ./util/palette.hpp \
    ./util/parms.hpp \
    ./util/point.hpp \
    ./util/range.hpp \
    ./util/static_assert.hpp \
    ./util/tensor.hpp \
    ./util/texture.hpp \
    ./util/vector.hpp \
    ./view/orthoviewer.hpp \
    ./view/perspviewer.hpp \
    ./view/viewer.hpp \
    ./external/gl2ps.h

MY_BASE  = ../..
include( $${MY_BASE}/common.pri )
