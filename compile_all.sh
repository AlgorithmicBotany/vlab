#! /bin/bash

makebin="make"
cmakebin="cmake"
config="./config_cache.sh"
findbin="find"

if ! [ -x ./compile_all.sh ]; then
  echo "Error, this script must be launched from the base directory of the VLab repository"
  exit -1
fi

if ! [ -x $qmakeosbin ]; then
  echo "Error, you must run the configure script before compiling VLab"
  exit -2
fi

source $config

MAKEOPTS=""

function cleanCompile() {
  echo "Cleaning vlab"
  if [ -e Makefile ]; then
    $makebin clean
  fi
  echo "Cleaning QGLViewer"
  if [ -e libQGLViewer/QGLViewer/Makefile ]; then
    $makebin -C libQGLViewer/QGLViewer clean
  fi
  echo "Cleaning VV/VVE"
  rm -Rf vve_build vv_build vvsf_build
  echo "Removing makefiles"
  IFS=$'\n' qmakefiles=($($findbin . -name \*.pro))
  for qm in ${qmakefiles[@]}; do
    m=$(dirname $qm)/Makefile
    if [ -e $pm ]; then
      rm -f $m
    fi
  done
#  if [ "$platform" == "Darwin" ]; then
    echo "Cleaning Rayshade"
    pushd .libraries
    if ! (rm -f *.a); then
      exit -1
#    fi
  fi
#
  echo "Cleaning done"
}

function compileBase() {
  configureBase
  if ! ($makebin $MAKEOPTS); then
    exit -1
  fi
  platform=$(uname)
  echo $platform
#  if [ "$platform" == "Darwin" ]; then
#      source ./config_cache.sh

#      echo "Arch = $CMAKE_OSX_ARCHITECTURES"
#      echo "$macVersion"
#     pushd Rayshade
#     rayshademakefile="Makefile.darwin"
#    if [ "$CMAKE_OSX_ARCHITECTURES" == "i386" ]; then
#	 echo "*** Making RayShade ***"
#	 rayshademakefile="Makefile.darwin.i386"
#    fi
#    if ! ($makebin -f $rayshademakefile $MAKEOPTS); then
#      exit -1
#    fi
#    echo -n "Copying Rayshade:"
#    if ! cp rayshade/rayshade ../.binaries/; then
#       echo -n "X"
#    else
#       echo -n "."
#   fi
 #   if ! cp raypaint/raypaint ../.binaries/; then
#      echo -n "X"
#   else
#      echo -n "."
#   fi
#   echo "OK"
#    popd
#  fi
}

function compileQGLViewer() {
# Compile the shared lib
  pushd libQGLViewer/QGLViewer
  if ! qmake_os; then
    exit -1
  fi
  if ! ($makebin $MAKEOPTS); then
    exit -1
  fi
  if ! ($makebin $MAKEOPTS install); then
    exit -1
  fi
# Then compile the static lib
  if ! (qmake_os QGLVIEWER_STATIC=1); then
    exit -1
  fi
  if ! ($makebin $MAKEOPTS); then
    exit -1
  fi
  if ! ($makebin $MAKEOPTS install); then
    exit -1
  fi
  popd
}

function compileWithCmake() {
  local project=$1
  local need_doc=$2
  local builddir=${project}_build
  if ! [ -d "$builddir" ]; then
    configureCMake $project
  fi
  if [ "$need_doc" == "doc" ]; then
    if ! [ -d "$builddir/doc" ]; then
      if ! ($makebin $MAKEOPTS -C "$builddir" doc); then
        exit -1
      fi
    fi
  fi
  if ! ($makebin $MAKEOPTS -C "$builddir"); then
    exit -1
  fi
  if ! ($makebin $MAKEOPTS -C "$builddir" install); then
    exit -1
  fi
  if ! ($makebin $MAKEOPTS -C "$builddir" install_doc); then
    exit -1
  fi
}

function compileVV() {
  echo "Compiling VV"
  compileWithCmake vv
#  compileWithCmake vvsf
}

function createVVeDoc() {
  $makebin $MAKEOPTS -C vve_build doc
}

function compileVVe() {
  compileWithCmake vve doc
}

function installAll() {
  $makebin $MAKEOPTS bindist
}

function makeStandalone() {
  installScript
}

function help() {
  echo "Usage: $0 [OPTION]"
  echo "    -a   Compile all"
  echo "    -b   Compile VLab base"
  echo "    -c   Clean all compilations"
  echo "    -d   Create doc for VVe"
  echo "    -e   Compile VVe"
  echo "    -i   Install all"
  echo "    -j n Compile with n processed"
  echo "    -q   Compile QGLViewer"
  echo "    -s   Make stand-alone package"
  echo "    -v   Compile VV"
  echo "    -h   Show this help"
}

wantBase=""
wantVVe=""
wantVV=""
wantQGLViewer=""
wantInstall=""
wantVVeDoc=""
wantClean=""
wantStandalone=""

args=$(getopt abcdeij:qvhs $*)
if [ $? != 0 ]; then
  echo "Error, didn't recognised arguments"
  help
  exit -3
fi
set -- $args
while true; do
  case "$1" in
    -a)
      wantBase="yes"
      wantVVe=""
      wantVV="yes"
      wantQGLViewer="yes"
      wantInstall="yes"
      ;;
    -b)
      wantBase="yes"
      ;;
    -c)
      wantClean="yes"
      ;;
    -d)
      wantVVeDoc="yes"
      ;;
    -e)
      wantVVe="yes"
      ;;
    -i)
      wantInstall="yes"
      ;;
    -j)
      MAKEOPTS="$MAKEOPTS -j $2"
      shift
      ;;
    -q)
      wantQGLViewer="yes"
      ;;
    -v)
      wantVV="yes"
      ;;
    -s)
      wantStandalone="yes"
      ;;
    -h)
      help
      exit 0
      ;;
    --)
      break
      ;;
  esac
  shift
done

if [ -z "$wantBase$wantVVe$wantVV$wantQGLViewer$wantInstall$wantClean$wantVVeDoc$wantStandalone" ]; then
  wantBase="yes"
  wantVVe=""
  wantVV=""
  wantQGLViewer=""
  wantInstall="yes"
fi

if ! [ -z "$wantClean" ]; then
  cleanCompile
fi

if ! [ -z "$wantBase" ]; then
  compileBase
fi

if ! [ -z "$wantQGLViewer" ]; then
  compileQGLViewer
fi

if ! [ -z "$wantVV" ]; then
  compileVV
fi

if ! [ -z "$wantVVe" ]; then
  compileVVe
fi

if ! [ -z "$wantVVeDoc" ]; then
  createVVeDoc
fi

if ! [ -z "$wantInstall" ]; then
  installAll
fi

if ! [ -z "$wantStandalone" ]; then
  makeStandalone
fi

    
