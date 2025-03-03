#!/bin/bash

rmbin="rm"
cpbin="cp"
mvbin="mv"
lnbin="ln"
tarbin="tar"
lsbin="ls"
mkdirbin="mkdir"
touchbin="touch"
installnametoolbin="install_name_tool"
otoolbin="otool"
PlistBuddybin="/usr/libexec/PlistBuddy"
grepbin="grep"
sedbin="sed"
qmakebin="qmake"
dittobin="ditto"

vlabversion=$(../.binaries/version.app/Contents/MacOS/version)
vlabbuild=$(../.binaries/version.app/Contents/MacOS/version -b)
vlabname="vlab-$vlabversion"
oldrootdir="$vlabname"
rootdir="$vlabname-$vlabbuild-with-qt"

rm -rf $rootdir

cp -r $vlabname $rootdir

# run macdeployqt to codesign all the binaries and gather Qt library files
# NOTE: you should be able to specify all executables here, but this doesn't seem to work well
# so, instead, it is done manually using the code from the old 'make-standalone-bundle.sh', copied below
macdeployqt $rootdir/browser.app -codesign=- #-executable=$rootdir/browser.app/Contents/System/vlabd.app/Contents/MacOS/vlabd
#-executable=$rootdir/browser.app/Contents/Plug-ins/lpfg.app/Contents/MacOS/lpfg


# load functions for resolving symlinks
. realpath.sh

# List the Qt libraries linked within this program
# call:
#  $ listQtLibs prg
# print on the standard output the list of Qt libraries linked to this 
# application
function listQtLibs() {
  if [ -z $1 ] ; then
    echo "listQtLibs called with no arguments :(" >&2
    exit -1
  fi
  name=$1
  local result=""
  IFS=$'\n' lines=($($otoolbin -L $name | $grepbin '\(Qt.*\.framework\|libQt.*.dylib\|phonon.framework\)' | $sedbin 's/[ 	]*\([^ 	][^ 	]*\).*/\1/'))
  for line in "${lines[@]}"; do
      result="$result:$line"
  done
  echo $name " --> " ${result:1} >&2
  echo ${result:1}
}

function isFramework() {
  if [ -z $1 ]; then
    echo "isFramework called with no arguments :(" >&2
    exit -1
  fi
  name="$1"
  if ($grepbin -q '.framework' <<< "$name"); then
    return 0
  else
    return 1
  fi
}

# Convert the list given as second argument into the array given as first 
# argument. As is traditionnal, the list is colon-separated
function listToArray() {
  IFS=':' read -ra $1 <<< "$2"
}

# Find the absolute path of a Qt library
function resolveQtLib() {
  local lib="$1"
  if [ "${lib:0:1}" == "/" ]; then
    realpath $lib
  else
    if isFramework "$lib"; then
      listToArray fmwk_paths "$fmwk_paths"
      for p in "${fmwk_paths[@]}"; do
        local cur_qt_lib="$p/$lib"
        if [ -e $cur_qt_lib ]; then
          realpath $cur_qt_lib
          return 0
        fi
      done
      return -2
    else
      listToArray lib_paths "$lib_paths"
      for p in "${lib_paths[@]}"; do
        local cur_qt_lib="$p/$lib"
        if [ -e $cur_qt_lib ]; then
          realpath $cur_qt_lib
          return 0
        fi
      done
      return -2
    fi
  fi
}

function extractFrameworkName() {
    echo "extractFrameworkName"
  if [ $# -eq 1 ]; then
    extractFrameworkName <<< "$1"
  else
    $sedbin -E -n 's,(.*/)*([^/]*\.framework|[^/]*\.dylib)(/.*)*,\2,p'
  fi
}

function extractFrameworkPath() {
  if [ $# -eq 1 ]; then
    extractFrameworkPath <<< "$1"
  else
    $sedbin -E -n 's,(.*/)*([^/]*\.framework|[^/]*\.dylib)(/.*)*,\2:\1\2,p'
  fi
}

# Find the name of the library as registered in the library itself
function findLibName() {
  local lib="$1"
  $otoolbin -L $lib | head -n 2 | tail -n 1 | $sedbin 's/[ 	]*\([^ 	][^ 	]*\).*/\1/' # Extract the second line
}

# Register the qt lib names globally in $qt_libs and current_qt_libs
function registerQtLib() {
    local l=$1
    IFS=':' read ln rp <<< "$(extractFrameworkPath "$l")"
    rp=${rp/@rpath/"$QTDIR/lib/"}
  local lp=$(resolveQtLib $rp)
  if [ -z "$lp" ]; then
    echo "Error, cannot resolve library $l"
    echo "  ln=$ln"
    echo "  rp=$rp"
    return -2
  fi
  if ($grepbin -Eiqv "(^|:)$ln($|:)" <<< "$qt_libs"); then
    qt_libs="$qt_libs:$ln"
    qt_libs_path="$qt_libs_path:$lp"
  elif ($grepbin -qiv "$lp" <<< "$qt_libs_path"); then
    echo ""
    echo "Error, the library $ln ($lp) has already been registered with a different path."
    echo "Current paths: $qt_libs_path"
    return -2
  fi
  if ($grepbin -Eiqv "(^|:)$ln($|:)" <<< "$current_qt_libs"); then
    current_qt_libs="$current_qt_libs:$ln"
  fi
}

# Convert a path to a global library into the path to the same library, but 
# private
function privatePath() {
  $sedbin -E -n 's,(.*/)*([^/]*\.framework/.*|[^/]*\.dylib),@executable_path/../Frameworks/\2,p' <<< "$1"
}

# Function that replaces all reference to Qt libs by local references and add 
# the replaced libs into current_qt_libs and qt_libs
function setupBinary() {
    local bin=$1
    local libs=$(listQtLibs "$bin")
    local name=$(basename $bin)
    listToArray libs_a "$libs"
    for l in "${libs_a[@]}"; do
	if (grep -q $name <<< "$l") || [ -z "$l" ]; then # Skip self-dependancy
	    continue
	fi
	if ! registerQtLib "$l"; then
	    return -2
	fi
	local pl=$(privatePath $l)
	if [ -n "$dont_write" ]; then
	    echo -n '.'
	else
	    if $installnametoolbin -change $l $pl $bin; then
                echo 1 codesign -s - -f "$bin"
                codesign -s - -f "$bin"
		echo -n "."
	    else
		echo "FAILED: $installnametoolbin -change $l $pl $bin"
		return -2
	    fi
	fi
    done
}

# Function that takes a framework and change any reference to Qt to a local one
function setupFramework() {
  local fmwk="$1"
  local fmwk_name="$(extractFrameworkName $fmwk | $sedbin 's/\.framework//')"
  if [ -z "$fmwk_name" ]; then
    echo "Error, cannot find the name of the framework $fmwk"
    return -2
  fi
# Scan all versions
  for v in $($lsbin -p $fmwk/Versions | $grepbin '/'); do
    for bin in $($lsbin $fmwk/Versions/$v | $grepbin "^$fmwk_name"); do
      if setupBinary "$fmwk/Versions/$v/$bin"; then
        echo -n ";"
      else
        return -2
      fi
    done
  done
}

function getBundleMainProgram() {
  bundle=$1
  info=$bundle/Contents/Info.plist
  if [ -e $info ]; then
    $PlistBuddybin -c "Print CFBundleExecutable" $info
  else
    echo "Error, bundle $bundle has no Info.plist file"
    return -2
  fi
}

function findFrameworks() {
  local bundle="$1"
  local fmwk_path="$1/Contents/Frameworks"
  if [ -d $fmwk_path ]; then
    $lsbin $fmwk_path | $sedbin -n "s,^.*.framework\$,$fmwk_path/&,p"
  fi
}

function findLibraries() {
  local bundle="$1"
  local libs_path="$1/Contents/Libraries"
  if [ -d $libs_path ]; then
    $lsbin $libs_path | $sedbin -n "s,^.*.dylib\$,$libs_path/&,p"
  fi
}

function linkFrameworks() {
  local libs=("$@")
  local bundle="${libs[0]}"
  if [ -z "$bundle" ]; then
    echo "Error, linkFrameworks() called with empty bundle. Args = '$@'"
    return -2
  fi
  local link="$bundle/Contents/Frameworks"
  if [ -n "$dont_write" ]; then
    echo -n '.'
  else
    if ($lnbin -sf ../../../Frameworks $link); then
      echo -n "!"
    else
      echo "Error, cannot create symbolic link '$link'"
      return -2
    fi
  fi
}

function linkPlugIns() {
  local libs=("$@")
  local bundle="${libs[0]}"
  if [ -z "$bundle" ]; then
    echo "Error, linkPlugIns() called with empty bundle. Args = '$@'"
    return -2
  fi
  local link="$bundle/Contents/PlugIns"
  if [ -n "$dont_write" ]; then
    echo -n '.'
  else
    if ($lnbin -sf ../../../PlugIns $link); then
      echo -n "!"
    else
      echo "Error, cannot create symbolic link '$link'"
      return -2
    fi
  fi
}

function setupBundle() {
  if [ -z $1 ] ; then
    echo "setupBundle called with no arguments :(" >&2
    exit -1
  fi
  local bundle="$1"
  local prg=$bundle/Contents/MacOS/$(getBundleMainProgram $bundle)
  current_qt_libs=""
  if setupBinary "$prg"; then
    echo -n '.'
  else
    return -2
  fi
  echo -n ";"
  for fmwk in $(findFrameworks $bundle); do
    if setupFramework "$fmwk"; then
      echo -n ";"
    else
      echo "FAILED to setup framework $fmwk"
      exit -3
    fi
  done
  for lib in $(findLibraries $bundle); do
    if setupBinary "$lib"; then
      echo -n ";"
    else
      echo "FAILED to setup library $lib"
      exit -3
    fi
  done
  linkFrameworks "$bundle" $current_qt_libs
# Now, link the Qt plugins and create the qt.conf
  if [ -z "$dont_write" ]; then
    if [ -e $bundle/Contents/PlugIns ] || [ -h $bundle/Contents/PlugIns ]; then
      if $rmbin $bundle/Contents/PlugIns; then
        echo -n '.'
      else
        echo "Error, cannot remove $bundle/Contents/PlugIns"
        return -2
      fi
    fi
    #if ( [ $1 != $raserverbundle ]); then
	echo "setup Bundle $1"
	echo -n
	if $lnbin -sf ../../../PlugIns $bundle/Contents/PlugIns; then
	    echo -n '.'
	else
	    echo "Error, cannot create link $bundle/Contents/PlugIns"
	    return -2
	fi
    #fi
    if ! [ -d $bundle/Contents/Resources ]; then
      $rmbin -f $bundle/Contents/Resources
      if $mkdirbin -p $bundle/Contents/Resources; then
        echo -n '.'
      else
        echo "Error, cannot create directory $bundle/Contents/Resources"
        return -2
      fi
    fi
    $rmbin -f $bundle/Contents/Resources/qt.conf
    cat > $bundle/Contents/Resources/qt.conf <<EOF
[Paths]
Plugins = PlugIns
Imports = Resources/qml
Qml2Imports = Resources/qml
EOF
  fi
}


# list of all bundles
pluginbundles=`find $rootdir/browser.app/Contents/Plug-ins -name '*.app' -type d -depth 1 -print`
echo bundles in plugins: $pluginbundles
  
systembundles=`find $rootdir/browser.app/Contents/System -name '*.app' -type d -depth 1 -print`
echo bundles in system: $systembundles

# Setup all bundles
for bundle in $pluginbundles $systembundles ; do
  echo -n "Setting up bundle $bundle: "
  if setupBundle $bundle; then
    echo "OK"
  else
    echo "FAILED"
    exit -1
  fi
done




echo "Copying oofs :"
pwd
mv -f $rootdir/browser.app/Contents/Resources/oofs $rootdir

echo "Copying Gifts :"
pwd
mv -f $rootdir/browser.app/Contents/Resources/Gifts $rootdir


echo "Copying README :"
pwd
cp -f README.mac $rootdir/README



if [ -d "$rootdir" ]; then
  echo -n "Renaming root directory: "
  if rm -Rf "$rootdir-native"; then
    echo -n '.'
  else
    echo "Error, cannot remove directory $rootdir-native"
    exit -2
  fi
  if mv "$rootdir" "$rootdir-native"; then
    echo "OK"
  else
    echo "Error, cannot rename directory $rootdir into $rootdir-native"
    exit -2
  fi
fi

  archname=native
  echo "Arch : $archname"
  archrootdir="$rootdir-$archname"
  # create a dmg file
  if [ -z "$dont_package" ]; then
    dmgfname="$vlabname-$vlabbuild-with-qt-$archname.dmg"
    echo -n "creating DMG $dmgfname..."
    rm -rf $dmgfname
    if hdiutil create -fs HFS+ -srcfolder $vlabname-$vlabbuild-with-qt-$archname -volname $vlabname-$vlabbuild-with-qt-$archname $dmgfname; then
      echo 'OK'
    else
      echo 'FAILED'
      exit 1
    fi
    echo `ls -l $dmgfname`
  fi



echo
echo "==========================================================="
echo "Bundle done."
echo "You may want to inspect the log: $deploylog"



