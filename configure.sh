#! /bin/bash

# remove the qmake stash file because when changes to the environmnet are made (new OSX, etc.)
# qmake projects need to pick up the change.
rm -f .qmake.stash

# detect platform
case "$(uname)" in
	'Linux' )
		platform='linux'
		;;
	'Darwin' )
		platform='osx'
		;;
	* )
		platform='unknown'
		;;
esac

# set tool names
makebin="make"
case $platform in
	'linux' )
		qmakebin='qmake'
		;;
	* )
		qmakebin="qmake"
		;;
esac
cmakebin="cmake"
findbin="find"

# Until we actually need cmake, there is no need to check if it is installed
#for tool in $qmakebin $makebin $cmakebin $findbin; do
for tool in $qmakebin $makebin $findbin; do
	if ! type $tool >/dev/null 2>&1; then
		echo "ERROR: \"$tool\" not found."
		echo "       This is needed by $scriptname to work. Check your"
		echo "       \$PATH variable or install the tool \"$tool\"."
		exit 2
	fi
done


if ! [ -x ./configure.sh ]; then
  echo "Error, this script must be launched from the base directory of the VLab repository"
  exit -1
fi

platform=$(uname)

CMAKEOPTS=()
QMAKEOPTS=()

function create_cache() {
  if [ ! -z "${CXX}" ]; then
    echo "CXX=${CXX}" >> config_cache.sh
    QMAKEOPTS=( "${QMAKEOPTS[@]}" "QMAKE_CXX=${CXX}" )
  fi
  if [ ! -z "${CC}" ]; then
    echo "CC=${CC}" >> config_cache.sh
    QMAKEOPTS=( "${QMAKEOPTS[@]}" "QMAKE_CC=${CC}" )
  fi
  echo "QMAKEOPTS=(${QMAKEOPTS[@]})" > config_cache.sh
  echo "CMAKEOPTS=(${CMAKEOPTS[@]})" >> config_cache.sh
  cat >> config_cache.sh <<EOF
function qmake_os() {
  $qmakebin "\${QMAKEOPTS[@]}" "\$@"
  return \$?
}

function configureBase() {
  if ! qmake_os; then
    exit -1
  fi
}

function installScript() {
  cd Distribution
  $INSTALLSCRIPT $INSTALLOPTION
}
EOF
  chmod +x config_cache.sh
}

function help() {
  echo "Usage: $0 [OPTION]"
  echo "    -A x Architecture setting: x should be one of 'arm64', 'x86', 'x86_64' or 'both' (x86 and x86_64). If not specified, will default to your compiler.'"
  echo "    -c   Clean all compilations"
  echo "    -M x Version of Max OS X to compile the system for (default: 10.15)"
  echo "    -h   Show this help"
}

function cleanCompile() {
  echo "Cleaning vlab"
  if [ -e Makefile ]; then
    $makebin clean
    rm -rf .binaries/*
  fi
  echo "Removing makefiles"
  IFS=$'\n' qmakefiles=($($findbin . -name \*.pro))
  for qm in ${qmakefiles[@]}; do
    m=$(dirname $qm)/Makefile
    if [ -e $pm ]; then
      rm -f $m
    fi
  done
  echo "Cleaning done"
}


macVersion="10.15"

wantClean=""

args=$(getopt A:cM:h $*)
if [ $? != 0 ]; then
  echo "Error, didn't recognised arguments"
  help
  exit -3
fi
set -- $args
while true; do
  case "$1" in
    -A)
      case "$platform" in
        "Linux")
          case "$2" in
            x86)
              QTARCHS="-m32"
              CMAKEARCHS="x86"
              ;;
            x86_64)
              QTARCHS="-m64"
              CMAKEARCHS="x86_64"
              ;;
            both)
              echo "Error, multiple architecture is not available on Linux"
              exit -2;
              ;;
            *)
              echo "Error, unknown arch '$2'"
              exit -1
              ;;
          esac
          ;;
          "Darwin")
          case "$2" in
            arm64)
              QTARCHS="arm64"
              CMAKEARCHS="arm64"
              INSTALLOPTION="--64"
	      rm config_lpfg.pri
	      echo "CONFIG += arm64" > config_lpfg.pri
              ;;

            x86)
              QTARCHS="x86"
              CMAKEARCHS="i386"
              INSTALLOPTION="--32"
	      rm config_lpfg.pri
	      echo "CONFIG += x86" > config_lpfg.pri
              ;;
            x86_64)
              QTARCHS="x86_64"
              CMAKEARCHS="x86_64"
              INSTALLOPTION="--64"
	      rm config_lpfg.pri
	      echo "CONFIG += x86_64" > config_lpfg.pri
              ;;
            both)
              QTARCHS="'x86 x86_64'"
              CMAKEARCHS="'i386;x86_64'"
              INSTALLOPTION="--both"
	      rm config_lpfg.pri
	      echo "CONFIG += x86 x86_64" > config_lpfg.pri
              ;;
            *)
              echo "Error, unknown arch '$2'"
              exit -1
              ;;
          esac
          ;;
          *)
          echo "Error, architecture specification is not supported for your platform '$platform'"
          exit -2;
      esac
          shift
          ;;
        -h)
          help
          exit 0
          ;;
        -M)
          macVersion="$2"
          shift
          ;;
        -c)
          wantClean="yes"
          ;;
        --)
          break
          ;;
      esac
  shift
done
installdir=$(pwd)/.binaries

case "$platform" in
  Darwin)
    echo "macOS deployment_target is '$macVersion'"
    deployment_target=$macVersion

    QMAKEOPTS+=(-spec macx-clang QMAKE_MAC_SDK=macosx QMAKE_MACOSX_DEPLOYMENT_TARGET=$deployment_target CONFIG+="$QTARCHS" CONFIG+="c++11")
    INSTALLSCRIPT="./vlab-macdeployqt.sh"
    ;;
  Linux)
    CMAKEOPTS+=(-D CMAKE_INSTALL_PREFIX="$installdir")
    if [ ! -z "${CMAKEARCHS}" ]; then
      CMAKEOPTS=(${CMAKEOPTS[@]} -D TARGET_SYSTEM_PROCESSOR="${CMAKEARCHS}")
    fi
    INSTALLSCRIPT=""
    INSTALLOPTION=""
    if [ ! -z "$QTARCHS" ]; then
      QMAKEOPTS+=(QMAKE_CXXFLAGS=$QTARCHS -stdlib=libc++ QMAKE_CFLAGS=$QTARCHS QMAKE_LFLAGS=$QTARCHS -stdlib=libc++  CONFIG+="c++11")
    fi
    ;;
  *)
    echo "Error, cannot compile for OS '$OS'"
    exit -2
    ;;
esac

if ! [ -z "$wantClean" ]; then
  cleanCompile
  exit 0
fi

create_cache

source ./config_cache.sh

configureBase

