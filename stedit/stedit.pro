# -------------------------------------------------
# Project created by QtCreator 2010-06-22T14:31:56
# -------------------------------------------------
QT += opengl
TARGET = stedit
TEMPLATE = app
SOURCES += Vector3.cpp \
    TurtleParametersWindow.cpp \
    Triangle.cpp \
    TranslateWindow.cpp \
    Trackball.cpp \
    TextureWindow.cpp \
    TextureEditor.cpp \
    Surface.cpp \
    RotateWindow.cpp \
    QImageFormatStrings.cpp \
    PointPicker.cpp \
    Point.cpp \
    Patch.cpp \
    Main.cpp \
    Edge.cpp \
    Colour.cpp \
    BezierWindow.cpp \
    BezierEditor.cpp \
    SurfaceViewport.cpp \
    ScaleWindow.cpp \
    AdjacenciesWindow.cpp \
    TextureEditorPreferencesDialog.cpp \
    BezierEditorPreferencesDialog.cpp \
    ResizeImageDialog.cpp \
    resources.cpp vector3d.cpp
HEADERS += Vector3.h \
    TurtleParametersWindow.h \
    Triangle.h \
    TranslateWindow.h \
    Trackball.h \
    TextureWindow.h \
    TextureEditor.h \
    Surface.h \
    RotateWindow.h \
    QImageFormatStrings.h \
    PointPicker.h \
    Point.h \
    Patch.h \
    Edge.h \
    Colour.h \
    BezierWindow.h \
    BezierEditor.h \
    SurfaceViewport.h \
    ScaleWindow.h \
    AdjacenciesWindow.h \
    TextureEditorPreferencesDialog.h \
    BezierEditorPreferencesDialog.h \
    ResizeImageDialog.h \
    resources.h  vector3d.h quaternion.h
OTHER_FILES += 
macx: {
LIBS += -framework Carbon
HEADERS +=		../libs/misc/cocoaBridge.h 
OBJECTIVE_SOURCES +=	../libs/misc/cocoaBridge.mm

LIBS +=		-framework AppKit
}
MY_BASE = ..


MY_LIBS = misc \
    platform
include( $${MY_BASE}/common.pri )
!isEmpty(MAKE_BUNDLE) { 
    # QMAKE_INFO_PLIST = Info.plist
    DEFINES += USE_PLUGINS
    LOCALDIR = $$DESTDIR/stedit.app/Contents/Resources/English.lproj
    HELPDIR = $$DESTDIR/stedit.app/Contents/Resources/stedit.help/Contents/Resources/English.lproj

    copyhelp.target = copyhelp
    copyhelp.commands = ( mkdir -p $$HELPDIR && cp -Rf '../docs/Quick_Help/SteditBezierQuickHelp.html' $$HELPDIR&& cp -Rf '../docs/Quick_Help/SteditWarpQuickHelp.html' $$HELPDIR && cp -rf ../docs/VLABToolsManual.pdf $$HELPDIR   && cp -rf ../docs/files/*.* $$HELPDIR)
    info_list.target = info_list
    info_list.commands = cp Info.plist $$DESTDIR/stedit.app/Contents/

    QMAKE_EXTRA_TARGETS += copyhelp info_list
    PRE_TARGETDEPS += copyhelp info_list
}
RESOURCES += resource.qrc ../libs/misc/about.qrc
macx::LIBS += -framework \
    Carbon
    #CONFIG += sanitizer sanitize_address

