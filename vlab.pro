TEMPLATE = subdirs
CONFIG += ordered
QT += widgets
QT_DISABLE_DEPRECATED_BEFORE=0x000000
SUBDIRS = \
libs \
vlab-splash \
vlabcpp \
bezieredit \
medit \
funcedit \
gallery \
object \
browser \
palette \
panels \
PreProc \
ped \
snapicon \
cuspy \
raserver \
cpfg \
l2c \
lpfg \
vlabd \
version \
ENVIRO \
stedit \
vlabTextEditor \
Timeline \ 
Rayshade \
vv

macx:SUBDIRS += updatebin

MY_BASE = .
include( common.pri )

mytarget.target = bindist
macx:{
  isEmpty(MAKE_BUNDLE) {
  } else {
    mytarget.commands = ( cd Distribution && $(MAKE) bindist-mac )
    mytarget2.target = deployqt
    mytarget2.commands = ( cd Distribution && $(MAKE) deployqt )
    mytarget2.depends = all
    QMAKE_EXTRA_TARGETS += mytarget2
    PRE_TARGETDEPS += mytarget2
  }
} 
unix:!macx:mytarget.commands = ( cd Distribution && $(MAKE) bindist-linux )
mytarget.depends = all
QMAKE_EXTRA_TARGETS += mytarget
PRE_TARGETDEPS += mytarget

