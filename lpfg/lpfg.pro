TEMPLATE = app
CONFIG  += qt opengl core 
SOURCES  = animparam.cpp colormap.cpp comlineparam.cpp configfile.cpp \
	   contour.cpp contourarr.cpp drawparam.cpp dynlib.cpp \
	   lsysdll.cpp environment.cpp envparams.cpp envturtle.cpp \
	   exception.cpp file.cpp funcs.cpp function.cpp gencyldata.cpp \
	   gencyltrtl.cpp glenv.cpp glturtle.cpp interface.cpp \
	   lengine.cpp lderive.cpp linterpret.cpp lightsrc.cpp lock.cpp \
	   lpfg.cpp lstring.cpp lstriter.cpp mainLnx.cpp material.cpp \
	   materialset.cpp mempool.cpp numchecktrtl.cpp \
	   objout.cpp objturtle.cpp patch.cpp pipepair.cpp  psout.cpp polygon.cpp \
	   povmesh.cpp povray.cpp povtrngl.cpp povrayturtle.cpp \
	   process.cpp projection.cpp psturtle.cpp rect.cpp rayshadeturtle.cpp \
	   semaphore.cpp succstor.cpp surface.cpp surfarr.cpp \
	   terrain.cpp terrainPatch.cpp texture.cpp texturearr.cpp \
	   tropism.cpp tropismarr.cpp \
	   tropismdata.cpp turtle.cpp utils.cpp viewLnx.cpp viewtask.cpp \
	   volume.cpp vvturtle.cpp vector3d.cpp winparams.cpp lpfgparams.cpp \
	   delay.cpp b_wrapper.cpp bsurfarr.cpp resources.cpp \
	   ContextScanner.cpp TextFileTurtle.cpp SaveAs.cpp  glwidget.cpp \
       mesharr.cpp mesh.cpp
           

HEADERS  = viewLnx.h SaveAs.h  glwidget.h

FORMS +=  SaveAs.ui 

QT += opengl printsupport widgets core

DEFINES  = LINUX DEBUG=0 GL_GLEXT_PROTOTYPES
TARGET   = lpfg
RESOURCES = lpfg.qrc ../libs/misc/about.qrc

INCLUDEPATH += ../libs
MY_BASE  = ..
MY_LIBS  = image shapes misc platform directoryWatcher

include( $${MY_BASE}/common.pri )

macx: { 
    include( $${MY_BASE}/config_lpfg.pri )
    LIBS += -framework GLUT -framework CoreFoundation -framework Carbon  -framework SystemConfiguration -liconv
    DEFINES += MACX_OPENGL_HEADERS

    # to speed-up testing, make install will copy the lpfg.app bundle to the Distribution directory
    # this is much faster than calling ./compile-all.sh from the root vlab directory
    # however, the vlab-x.x.x, version number has to be updated manually in the next line
    lpfg_quick_install.path = ../Distribution/vlab-5.0/browser.app/Contents/Plug-ins/
    lpfg_quick_install.files = ../.binaries/lpfg.app
    INSTALLS += lpfg_quick_install
}
else {
    linux-g++|linux-g++-64|linux {
        contains(QMAKE_CFLAGS, "-m32") {
            TO_COMPILE="32"
        } else: contains(QMAKE_CFLAGS, "-m64") {
            TO_COMPILE="64"
        } else {
            SYSTEM = $$system(gcc -v 2>&1)
            contains(SYSTEM, "x86_64-linux-gnu") { TO_COMPILE="64" }
        else { TO_COMPILE="32" }
        }
        contains(TO_COMPILE, "64") { LIBS += -lglut -ldl }
        else { LIBS += -lglut  -ldl}
    } else { LIBS += -lglut }

    # lpfg quick install to distribution for testing
    lpfg_quick_install.path = ../Distribution/vlab-5.0/bin/
    lpfg_quick_install.files = ../.binaries/bin/lpfg
    INSTALLS += lpfg_quick_install 
  }



# Add copying of ressources for mac
!isEmpty(MAKE_BUNDLE) {
#  QMAKE_INFO_PLIST = Info.plist

  LOCALDIR = $$DESTDIR/lpfg.app/Contents/Resources/English.lproj
  HELPDIR = $$DESTDIR/lpfg.app/Contents/Resources/LPFG.help/Contents/Resources/English.lproj

  copyhelp.target = copyhelp
  copyhelp.commands = mkdir -p $$HELPDIR && cp -f ../docs/LPFGManual.pdf $$HELPDIR && cp -f ../docs/Quick_Help/LPFGQuickHelp.html $$HELPDIR && cp -rf ../docs/files/*.* $$HELPDIR
  info_list.target = info_list
  info_list.commands = cp Info.plist $$DESTDIR/lpfg.app/Contents/
  doc_info_list.target = doc_info_list
  doc_info_list.commands = cp doc_Info.plist $$HELPDIR/../../Info.plist

#  l2cscripts.target = l2cscripts
# modif from PBdR
  l2cscripts.target = StdModulesStruct.h

  l2cscripts.commands = $$DESTDIR/l2c.app/Contents/MacOS/l2c include/stdmods.h StdModulesStruct.h -ModulesOnly


  QMAKE_EXTRA_TARGETS +=copyViewH copyViewCPP copyViewIMP copyLpfgH copyLpfgCPP copyLpfgIMP  copyTurtleH copyGLTurtleCPP copyLEngineH  copyhelp info_list doc_info_list l2cscripts


  #PRE_TARGETDEPS += copyhelp info_list doc_info_list l2cscripts
  # modif from PBdR
  PRE_TARGETDEPS += copyViewH copyViewCPP copyViewIMP copyLpfgH copyLpfgCPP copyLpfgIMP copyTurtleH copyGLTurtleCPP copyLEngineH copyhelp info_list doc_info_list StdModulesStruct.h

  # For some reason, optimisation for size crashes, but not optim for speed!
  QMAKE_CXXFLAGS_RELEASE -= -Os
  QMAKE_CXXFLAGS_RELEASE += -O3

  SOURCES += bundle.cpp
  DEFINES += MAKE_BUNDLE

  LPFG_HEADERS = include/lintrfc.h include/lparams.h include/lpfgall.h include/lsys.h include/stdmods.h
  LPFG_DESTDIR = $$DESTDIR/lpfg.app/Contents
  copyheaders.target = copyheaders
  copyheaders.depends = $$LPFG_HEADERS
  copyheaders.commands = mkdir -p  $$LPFG_DESTDIR/Resources/include && cp -p $$LPFG_HEADERS $$LPFG_DESTDIR/Resources/include
  message($$CONFIG)
  ARCH=""
  contains(CONFIG,x86) {
    ARCH+=" -arch i386"
  }
  contains(CONFIG,x86_64) {
    ARCH+=" -arch x86_64"
  }

  LPFG_SCRIPTS = scripts/Plug-in/cmpl.sh scripts/Plug-in/preproc.sh
  copyscripts.target = copyscripts
  copyscripts.depends = $$LPFG_SCRIPTS
  copyscripts.commands = chmod a+x $$LPFG_SCRIPTS;
#  copyscripts.commands +=  mkdir -p $$LPFG_DESTDIR/MacOS && cp -p $$LPFG_SCRIPTS $$LPFG_DESTDIR/MacOS;
  copyscripts.commands +=  mkdir -p $$LPFG_DESTDIR/MacOS && sed -e \'s/-ARCH/$${ARCH}/\' scripts/Plug-in/preproc.sh > $$LPFG_DESTDIR/MacOS/preproc.sh;
  copyscripts.commands +=  mkdir -p $$LPFG_DESTDIR/MacOS && sed -e \'s/-ARCH/$${ARCH}/\' scripts/Plug-in/cmpl.sh > $$LPFG_DESTDIR/MacOS/cmpl.sh;


  copymak.target = copymak
  copymak.depends = scripts/lpfg.mak.Mac
  copymak.commands = ( mkdir -p $$LPFG_DESTDIR/Resources && sed -e \'s/-ARCH/$${ARCH}/\' scripts/lpfg.mak.Mac > $$LPFG_DESTDIR/Resources/lpfg.mak )


  LPFG_SHADERS = shaders/main_vshader.glsl shaders/main_fshader.glsl shaders/shadow_vshader.glsl shaders/shadow_fshader.glsl
  copyshaders.target = copyshaders
  copyshaders.depends = $$LPFG_SHADERS
  copyshaders.commands = mkdir -p $$LPFG_DESTDIR/Resources/shaders && cp -p $$LPFG_SHADERS $$LPFG_DESTDIR/Resources/shaders;


  QMAKE_EXTRA_TARGETS += copyscripts copyheaders copymak copyshaders

  PRE_TARGETDEPS += copyscripts copyheaders copymak copyshaders

}

else {
#  l2cscripts.target = l2cscripts
# modif from PBdR
  l2cscripts.target = StdModulesStruct.h
  l2cscripts.commands = ../l2c/l2c include/stdmods.h StdModulesStruct.h -ModulesOnly

  LPFG_HEADERS = include/lintrfc.h include/lparams.h include/lpfgall.h include/lsys.h include/stdmods.h
  copyheaders.target = copyheaders
  copyheaders.depends = $$LPFG_HEADERS
  copyheaders.commands = mkdir -p  ../.binaries/include && cp -p $$LPFG_HEADERS ../.binaries/include

  LPFG_SHADERS = shaders/main_vshader.glsl shaders/main_fshader.glsl shaders/shadow_vshader.glsl shaders/shadow_fshader.glsl

  copyshaders.target = copyshaders
  copyshaders.depends = $$LPFG_SHADERS
  copyshaders.commands = mkdir -p ../.binaries/share/shaders && cp -p $$LPFG_SHADERS ../.binaries/share/shaders;

  QMAKE_EXTRA_TARGETS += l2cscripts copyheaders copyshaders

#  PRE_TARGETDEPS = l2cscripts
# modif from PBdR
  PRE_TARGETDEPS = StdModulesStruct.h copyheaders copyshaders
}


# the lpfg manual is copied from the 'docs' directory
# so this is not needed
#unix:!macx{
#LOCALDIR = $$DESTDIR/../
#LPFG_HELPDIR = $$HELPDIR/lpfg/
#mkdir.commands = ($$QMAKE_MKDIR $$LPFG_HELPDIR)
#mkdir.target = $$LPFG_HELPDIR
#copyhelp.target = copyhelp
#copyhelp.commands = ( mkdir -p $$LPFG_HELPDIR && cp -rf 'LPFGManual/LPFGHelp/*' $$LPFG_HELPDIR && cp -f LPFGManual.pdf $$LPFG_HELPDIR )
#QMAKE_EXTRA_TARGETS +=  copyhelp info_list doc_info_list
#PRE_TARGETDEPS +=   copyhelp info_list doc_info_list
#}
