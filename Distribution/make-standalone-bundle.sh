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
scriptname="make-standalone-bundle.sh"
qmakebin="qmake"
dittobin="ditto"

vlabversion=$(../.binaries/version.app/Contents/MacOS/version)
vlabbuild=$(../.binaries/version.app/Contents/MacOS/version -b)
vlabname="vlab-$vlabversion"
oldrootdir="$vlabname"
rootdir="$vlabname-$vlabbuild-with-qt"

dont_write=""

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
  $sedbin -E -n 's,(.*/)*([^/]*\.framework/.*|[^/]*\.dylib),@executable_path/../QtFrameworks/\2,p' <<< "$1"
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
  local link="$bundle/Contents/QtFrameworks"
  if [ -n "$dont_write" ]; then
    echo -n '.'
  else
    if ($lnbin -sf ../../../QtFrameworks $link); then
      echo -n "!"
    else
      echo "Error, cannot create symbolic link '$link'"
      return -2
    fi
  fi
#  local fmwks="$bundle/Contents/Frameworks"
#  if [ -e $fmwks ] && ! [ -d $fmwks ]; then
#    if ! $rmbin -Rf $fmwks; then
#      echo "Error, could not erase non-directory $fmwks"
#      return -2
#    fi
#  fi
#  if ! [ -e $fmwks ]; then
#    if ! $mkdirbin -p $fmwks; then
#      echo "Error, could not create directory $fmwks"
#      return -2
#    fi
#  fi
#  for ((i=1 ; i < ${#libs[@]} ; i++)); do
#    if $lnbin -sf "../../../../Frameworks/${libs[i]}" "$fmwks/${libs[i]}"; then
#      echo -n '.'
#    else
#      echo "Error, could not create link to $fmwks/${libs[i]}"
#      exit 2
#    fi
#  done
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
    if ( [ $1 != $raserverbundle ]); then
	echo "setup Bundle $1"
	echo -n
	if $lnbin -sf ../../../PlugIns $bundle/Contents/PlugIns; then
	    echo -n '.'
	else
	    echo "Error, cannot create link $bundle/Contents/PlugIns"
	    return -2
	fi
    fi
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
Prefix=./
EOF
  fi
}

function help() {
  echo "Usage: ./$scriptname [OPTIONS]"
  echo "   --nocopy     Don't copy the bundle, assume it is already at the right place (i.e. in vlab-4.2.14-with-qt)"
  echo "   --dry-run    Dry run (don't do anything, just check the Qt libraries"
  echo "   --nopackage  Don't create the package (.dmg)"
  echo "   --justpackage Just create the package(s) from an existing vlab-***-with-qt"
  echo "   --qmake=QMAKE_PATH     Specify the path to qmake (useful if not using the default one)"
  echo "   --32         Keep only 32 bits code"
  echo "   --64         Keep only 64 bits code"
  echo "   --split      Equivalent to '--32 --64'"
  echo "   --native     Package the native architecture (i.e. no stripping occurs)"
  echo "   --allarchs   Equivalent to '--32 --64 --native'"
  echo "   -h,--help   This help"
}

function tuneDistribution() {
  dir=$1
  arch=$2
  #files="browser.app/Contents/Plug-ins/vveinterpreter.app/Contents/Resources/makefile.in browser.app/Contents/Plug-ins/vvinterpreter.app/Contents/Resources/program.mk browser.app/Contents/Plug-ins/lpfg.app/Contents/Resources/lpfg.mak"
#  files=" browser.app/Contents/Plug-ins/vvinterpreter.app/Contents/Resources/program.mk browser.app/Contents/Plug-ins/lpfg.app/Contents/Resources/lpfg.mak"
  possible_archs="i386 x86_64 ppc ppc64"
  for f in $files; do
    fp="$dir/$f"
    for a in $possible_archs; do
      if [ "$a" != "$arch" ]; then
        if ! ($sedbin -e "s/-arch $a//g" "$fp" > "$fp.tmp"); then
          echo "Failed to remove architecture $a"
          return -1
        fi
        if ! $rmbin "$fp"; then
          echo "Failed to remove file $fp"
          return -1
        fi
        if ! $mvbin "$fp.tmp" "$fp"; then
          echo "Failed to rename file $fp.tmp into $fp"
          return -1
        fi
      fi
    done
  done
}

dont_package=""
dry_run=""
dont_copy=""
keep_arch=""
just_package=""
#args=$(getopts cdpq:h --long nocopy,dry-run,nopackage,qmake:,help -- "$@")
#if [ $? != 0 ]; then
#  echo "Error, didn't recognised arguments"
#  help
#  exit -3
#fi
#set -- $args
while true; do
  if [ -z "$1" ]; then
    break
  fi
  case "$1" in
    --nocopy)
      dont_copy="yes"
      ;;
    --dry-run)
      dry_run="yes"
      dont_copy="yes"
      dont_package="yes"
      dont_write="yes"
      rootdir="$oldrootdir"
      ;;
    --nopackage)
      dont_package="yes"
      ;;
    --justpackage)
      just_package="yes"
      dont_copy="yes"
      ;;
    --qmake)
      qmakebin="$2"
      if ! [ -f $qmakebin ] || ! [ -x $qmakebin ]; then
        echo "Error, specified qmake doesn't exist or is not an executable file"
        exit -2
      fi
      shift
      ;;
    -h|--help)
      help
      exit 0
      ;;
    --32)
      keep_arch="$keep_arch:i386"
      ;;
    --64)
      keep_arch="$keep_arch:x86_64"
      ;;
    --native)
      keep_arch="$keep_arch:native"
      ;;
    --split)
      keep_arch="$keep_arch:i386:x86_64"
      ;;
    --allarchs)
      keep_arch="native:i386:x86_64"
      ;;
    --)
      break
      ;;
    *)
      echo "Error, unrecognised option $1"
      help
      exit -1
      ;;
  esac
  shift
done

if [ -z $keep_arch ]; then
  keep_arch="native"
fi

# make sure we have the tools we need
for tool in find basename $cpbin $lsbin $rmbin $mkdirbin $deployqtbin $mvbin $lnbin $touchbin $installnametoolbin $otoolbin $PlistBuddybin $grepbin $sedbin $qmakebin $dittobin; do
	if ! type $tool >/dev/null 2>&1; then
		echo "ERROR: \"$tool\" not found."
		echo "       This is needed by $scriptname to work. Check your"
		echo "       \$PATH variable or install the tool \"$tool\"."
		exit 2
	fi
done

browserbundle=$rootdir/browser.app
raserverbundle=$rootdir/raserver.app
browserplugindir=$browserbundle/Contents/PlugIns

QT_LIBS=$($qmakebin -query QT_INSTALL_LIBS)
QT_PLUGINS=$($qmakebin -query QT_INSTALL_PLUGINS)
QT_BINS=$($qmakebin -query QT_INSTALL_BINS)
fmwk_paths="$QT_LIBS:/Library/Frameworks:/System/Library/Frameworks"
lib_paths="$QT_LIBS:/usr/lib:/usr/local/lib:$DYLD_LIBRARY_PATH"

# make sure we are in the right directory
if [ ! -f $scriptname ]; then
  echo "You are in the wrong directory. Please 'cd' to the directory where"
  echo "this script is located."
  exit 1
fi

echo "Making standalone bundle for mac."
echo "================================="
echo
echo "This script will make vlab bundle where QT is included as a private framework."
echo "All executables, i.e. browser.app and System/*.app and Plug-ins/*.app"
echo "will use this internal QT so the user does not need to install his own."
echo
echo "Assuming you already built bindist in:"
echo "    $oldrootdir"
echo -n "Let's check: "

# make sure the browser.app directory exists
if [ ! -d $oldrootdir/browser.app ]; then
    echo "Nope."
    echo "Did you forget to run make bindist?"
    exit 1
fi
echo "OK"

if [ -z "$dont_copy" ]; then
  # delete the old distribution
  if [ -d $rootdir ] ; then
    echo -n "Deleting previous distribution: "
    if $rmbin -rf $rootdir ; then
      echo "OK"
    else
      echo "FAILED"
      exit 1
    fi
  fi

  # make the new root
  echo "Making the new distro in:"
  echo "   $rootdir"
  if $mkdirbin -p $rootdir ; then
    echo "OK"
  else
    echo "FAILED"
    exit 1
  fi

  # copy over browser.app and raserver.app
  for dir in browser.app raserver.app 
  do
    echo -n "Copying $dir : "
    if $cpbin -Rp $oldrootdir/$dir $rootdir/$dir ;  then
      echo OK
    else
      echo FAILED
      echo "$cpbin -Rp $oldrootdir/$dir $rootdir/$dir"
      exit -1
    fi
  done

  # Copy the moc program
  echo -n "Copying moc program to system plugins : "
  if $cpbin $QT_BINS/moc $rootdir/browser.app/Contents/System; then
    echo OK
  else
    echo FAILED
    echo "$cpbin $QT_BINS/moc $rootdir/browser.app/Contents/System"
    exit -1
  fi
fi

if [ -z "$just_package" ]; then
  # list of all bundles
  pluginbundles=`find $rootdir/browser.app/Contents/Plug-ins -name '*.app' -type d -depth 1 -print`
  echo bundles in plugins: $pluginbundles
  
  systembundles=`find $rootdir/browser.app/Contents/System -name '*.app' -type d -depth 1 -print`
  echo bundles in system: $systembundles

  qt_libs=""
  qt_libs_path=""

  # Setup all bundle
  for bundle in $browserbundle $pluginbundles   $systembundles $raserverbundle; do
    echo -n "Setting up bundle $bundle: "
    if setupBundle $bundle; then
      echo "OK"
    else
      echo "FAILED"
      exit -1
    fi
  done

  echo "Browser plugin dir = '$browserplugindir'"

  # Copy Qt plugins and process them
  echo -n "Copying Qt plugins: "
  if [ -z "$dry_run" ]; then
    if [ ! -d "$browserplugindir" ]; then
      if $rmbin -Rf $browserplugindir; then
        echo -n '.'
      else
        echo "Error, cannot remove $browserplugindir"
        exit -2
      fi
      if $mkdirbin -p "$browserplugindir"; then
        echo -n "."
      else
        echo "Error, cannot create directory $browserplugindir"
        exit -2
      fi
    fi
  fi

  plugins="accessible codecs imageformats platforms styles"
  for p in $plugins; do
    if [ -z "$dry_run" ]; then
      if [ -d $browserplugindir/$p ]; then
        if $rmbin -Rf $browserplugindir/$p; then
          echo -n '.'
        else
          echo "Error, cannot erase directory $browserplugindir/$p"
        fi
      fi
      if $cpbin -Rf $QT_PLUGINS/$p $browserplugindir/$p; then
        echo -n "."
      else
        echo "Error, cannot copy plugin directory $QT_PLUGINS/$p to $browserplugindir/$p"
      fi
      for i in $browserplugindir/$p/*.dylib; do
        if setupBinary "$i"; then
          echo -n ';'
        else
          exit -2
        fi
      done
    else
      for i in $QT_PLUGINS/$p/*.dylib; do
        if setupBinary "$i"; then
          echo -n ';'
        else
          exit -2
        fi
      done
    fi
  done
  echo "OK"

  qt_libs="${qt_libs:1}"
  qt_libs_path="${qt_libs_path:1}"

  # First, scan all Qt libs for inter-dependancies
  echo -n "Checking for Qt libraries required by browser.app: "
  dont_write="yes"
  old_qt_libs=""
  while [ "$old_qt_libs" != "$qt_libs" ]; do
    listToArray qt_libs_a "$qt_libs"
    listToArray old_qt_libs_a "$old_qt_libs"
    listToArray qt_libs_path_a "$qt_libs_path"
    for (( i=0 ; i < ${#qt_libs_a[@]} ; i++ )); do
      l="${qt_libs_a[i]}"
      new="true"
      for j in "${old_qt_libs_a[@]}"; do
        if [ "$l" == "$j" ]; then
          new=""
          break
        fi
      done
      if [ -z "$new" ]; then
        continue
      fi
      cur_qt_lib="${qt_libs_path_a[i]}"
      if isFramework "$cur_qt_lib"; then
        if setupFramework "$cur_qt_lib"; then
          echo -n '.'
        else
          echo "Error, failed to analyse $cur_qt_lib"
          exit -2
        fi
      else
        if setupBinary "$cur_qt_lib"; then
          echo -n '.'
        else
          echo "Error, failed to analyse $cur_qt_lib"
          exit -2
        fi
      fi
    done
    old_qt_libs="$qt_libs"
  done
  if [ -z "$dry_run" ]; then
    dont_write=""
  fi
  echo "OK"

  # Now copy the Qt libraries and update them
  echo "Found Qt libraries   :"
  
  listToArray qt_libs_a "$qt_libs"
  listToArray qt_libs_path_a "$qt_libs_path"


 #     echo "Add LibQCocoa, apparently this is needed from QT5, but it's not documented"
 # $cpbin -R "$QTDIR/plugins/platforms/libqcocoa.dylib" "$browserbundle/Contents/QtFrameworks/" && chmod -R +w "$sharedframeworks/libcocoa.dylib"
#  $installnametoolbin -id "$QTDIR/plugins/platforms/libqcocoa.dylib" "$sharedframeworks/libqcocoa.dylib";
   #qt_libs_a=(${qt_libs_a[@]} "libqcocoa.dylib")
   #qt_libs_path_a=(${qt_libs_path_a[@]} "$QTDIR/plugins/platforms/libqcocoa.dylib")

  
  for (( i=0 ; i < ${#qt_libs_a[@]} ; i++ )); do
    echo "  ${qt_libs_a[i]} -> ${qt_libs_path_a[i]}"
  done

# raserver doesn't need Qt anymore
#  echo "List of Qt libraries required by $raserverbundle: "
#  qt_libs_ra=$(listQtLibs "../.binaries/raserver.app/Contents/MacOS/raserver")
#  qt_libs_raserver="$(extractFrameworkName $qt_libs_ra | $sedbin 's/\.framework//').framework"
#  listToArray qt_libs_b "${qt_libs_raserver}"
#  qt_libs_raserver_path="/Library/Frameworks/$qt_libs_raserver"
#  listToArray qt_libs_path_b "${qt_libs_raserver_path}"

#  listToArray qt_libs_path_b "$qt_libs_path_raserver"
echo "*****"
  for (( i=0 ; i < ${#qt_libs_b[@]} ; i++ )); do
      echo "  ${qt_libs_b[i]} -> ${qt_libs_path_b[i]}"
  done

  if [ -z "$dry_run" ]; then

    echo "Copy and install the Qt libraries into $browserbundle/Contents/QtFrameworks: "

    sharedframeworks=$browserbundle/Contents/QtFrameworks
    raserverqtframeworks=$raserverbundle/Contents/QtFrameworks
    if ( [ -e $sharedframeworks ] || [ -h $sharedframeworks ] ); then
      if $rmbin -Rf $sharedframeworks; then
        echo -n '.'
      else
        echo "Error, cannot erase $sharedframeworks"
        exit -2
      fi
      if $mkdirbin -p $sharedframeworks; then
        echo -n '.'
      else
        echo "Error, cannot create directory $sharedframeworks"
        exit -2
      fi
    fi
    if ( [ -e $raserverqtframeworks ] || [ -h $rasereverqtframeworks ] ); then
      if $rmbin -Rf $raserverqtframeworks; then
        echo -n '.'
      else
        echo "Error, cannot erase $raserverqtframeworks"
        exit -2
      fi
      if $mkdirbin -p $raserverqtframeworks; then
        echo -n '.'
      else
        echo "Error, cannot create directory $raserverqtframeworks"
        exit -2
      fi
    fi
  fi
  


  for (( i=0 ; i < ${#qt_libs_a[@]} ; i++ )); do
    lib="${qt_libs_a[i]}"
    cur_qt_lib="${qt_libs_path_a[i]}"
    local_qt_lib="$browserbundle/Contents/QtFrameworks/$lib"
    raserver_local_qt_lib="$raserverbundle/Contents/QtFrameworks/$lib"
   if [ -e "$cur_qt_lib" ]; then
      if [ ! -d $local_qt_lib ]; then
        if [ -z "$dry_run" ]; then
          if isFramework "$cur_qt_lib"; then
            if $cpbin -R "$cur_qt_lib" "$local_qt_lib" && chmod -R +w "$local_qt_lib"; then
              echo -n "."
            else
              echo "Error, cannot copy Qt library $cur_qt_lib into $local_qt_lib"
              exit -2
            fi
         else
            if $cpbin -L "$cur_qt_lib" "$local_qt_lib" && chmod -R +w "$local_qt_lib"; then
              echo -n "."
            else
              echo "Error, cannot copy Qt library $cur_qt_lib into $local_qt_lib"
              exit -2
            fi
         fi
        else
          local_qt_lib="$cur_qt_lib"
        fi
        if isFramework "$local_qt_lib"; then
          if setupFramework "$local_qt_lib"; then
            # If I could setup the framwork, then change its name
            for v in $(ls $local_qt_lib/Versions); do
              version="$local_qt_lib/Versions/$v"
              if [ -d "$version" ]; then
                for l in $(ls $version); do
                  lib="$version/$l"
                  if [ -f "$lib" ]; then
                    local_path=$(privatePath $(findLibName "$lib"))
                    if [ -z "$dry_run" ]; then
                      if $installnametoolbin -id "$local_path" "$lib"; then
                        echo -n '.'
                      else
                        echo "Cannot change the name of $lib to $local_path"
                        exit -2
                      fi
                    else
                      echo -n '.'
                    fi
                  fi
                done
              fi
            done
          else
            exit -2
          fi
        else
          if setupBinary "$local_qt_lib"; then
            local_path=$(privatePath $(findLibName "$local_qt_lib"))
            if [ -z "$dry_run" ]; then
              if $installnametoolbin -id "$local_path" "$local_qt_lib"; then
                echo -n '.'
              else
                echo "Cannot change the name of $local_qt_lib to $local_path"
                exit -2
              fi
            else
              echo -n '.'
            fi
          fi
        fi
        echo -n "."
      else
        echo "Error, Qt library $cur_qt_lib is not on your system."
        exit -2
      fi
    fi
    echo -n ';'
  done
  echo OK
  find $browserbundle/Contents/QtFrameworks/ -name "*debug*" -exec rm -rf "{}" \;
  echo "Install QtLibraries for $raserverbundle"
  for (( i=0 ; i < ${#qt_libs_b[@]} ; i++ )); do
    lib="${qt_libs_b[i]}"
    cur_qt_lib="${qt_libs_path_b[i]}"
    local_qt_lib="$raserverbundle/Contents/QtFrameworks/$lib"
   if [ -e "$cur_qt_lib" ]; then
      if [ ! -d $local_qt_lib ]; then
        if [ -z "$dry_run" ]; then
          if isFramework "$cur_qt_lib"; then
            if $cpbin -R "$cur_qt_lib" "$local_qt_lib" && chmod -R +w "$local_qt_lib"; then
              echo -n "."
 	      # this is to remove any debug symbols from the distribution
	      find $raserverbundle/Contents/QtFrameworks/ -name "*debug*" -exec rm -rf "{}" \;
              echo -n "."
           else
              echo "Error, cannot copy Qt library $cur_qt_lib into $local_qt_lib"
              exit -2
            fi

          else
            if $cpbin -L "$cur_qt_lib" "$local_qt_lib" && chmod -R +w "$local_qt_lib"; then
              echo -n "."
 	      # this is to remove any debug symbols from the distribution
	      find $raserverbundle/Contents/QtFrameworks/ -name "*debug*" -exec rm -rf "{}" \;
            else
              echo "Error, cannot copy Qt library $cur_qt_lib into $local_qt_lib"
              exit -2
            fi
         fi
        else
          local_qt_lib="$cur_qt_lib"
        fi
        if isFramework "$local_qt_lib"; then
          if setupFramework "$local_qt_lib"; then
            # If I could setup the framwork, then change its name
            for v in $(ls $local_qt_lib/Versions); do
              version="$local_qt_lib/Versions/$v"
              if [ -d "$version" ]; then
                for l in $(ls $version); do
                  lib="$version/$l"
                  if [ -f "$lib" ]; then
                    local_path=$(privatePath $(findLibName "$lib"))
                    if [ -z "$dry_run" ]; then
                      if $installnametoolbin -id "$local_path" "$lib"; then
                        echo -n '.'
                      else
                        echo "Cannot change the name of $lib to $local_path"
                        exit -2
                      fi
                    else
                      echo -n '.'
                    fi
                  fi
                done
              fi
            done
          else
            exit -2
          fi
        else
          if setupBinary "$local_qt_lib"; then
            local_path=$(privatePath $(findLibName "$local_qt_lib"))
            if [ -z "$dry_run" ]; then
              if $installnametoolbin -id "$local_path" "$local_qt_lib"; then
                echo -n '.'
              else
                echo "Cannot change the name of $local_qt_lib to $local_path"
                exit -2
              fi
            else
              echo -n '.'
            fi
          fi
        fi
        echo -n "."
      else
        echo "Error, Qt library $cur_qt_lib is not on your system."
        exit -2
      fi
    fi
    echo -n ';'
  done
  echo OK
  find $raserverbundle/Contents/QtFrameworks/ -name "*dSYM" -exec rm -rf "{}" \;
  find $raserverbundle/Contents/QtFrameworks/ -name "*debug*" -exec rm -rf "{}" \;

fi

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

listToArray keep_arch_a "${keep_arch}"
for arch in "${keep_arch_a[@]}"; do
  case "$arch" in
    i386)
    archname=32bits
    ;;
    x86_64)
    archname=64bits
    ;;
    native)
    archname=native
    ;;
    *)
    continue
    ;;
  esac
  echo "Arch : $archname"
  archrootdir="$rootdir-$archname"
  if [ "$arch" != "native" ]; then
    echo -n "Stripping program from code for architectures different from $arch: "
    if $rmbin -Rf "$archrootdir"; then
      echo -n '.'
    else
      echo FAILED to erase directory $archrootdir
    fi
    if $dittobin --arch $arch "$rootdir-native" "$archrootdir"; then
      echo -n '.'
    else
      echo FAILED
      exit -2
    fi
    if tuneDistribution "$archrootdir" "$arch"; then
      echo OK
    else
      echo FAILED tuning
      exit -2
    fi
  fi
  # create a dmg file
  if [ -z "$dont_package" ]; then
    dmgfname="$vlabname-$vlabbuild-with-qt-$archname.dmg"
    echo -n "creating DMG $dmgfname..."
    #if the size of the dmg is over 100M hidutil miscalulate the size, we have to calculate this size by hand
    #SIZE=`du -sh "$dmgfname" | sed 's/\([0-9]*\)M\(.*\)/\1/'`
    #SIZE=`echo "${SIZE} * 2.0" | bc | awk '{print int($1+0.5)}'`
    #    if hdiutil create $dmgfname -srcfolder $archrootdir -ov ; then
    rm -rf $dmgfname
    if hdiutil create -fs HFS+ -srcfolder $vlabname-$vlabbuild-with-qt-$archname -volname $vlabname-$vlabbuild-with-qt-$archname $dmgfname; then
      echo 'OK'
    else
      echo 'FAILED'
      exit 1
    fi
    echo `ls -l $dmgfname`
  fi
done



echo
echo "==========================================================="
echo "Bundle done."
echo "You may want to inspect the log: $deploylog"

