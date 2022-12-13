#!/bin/sh

vlabversion=$(../.binaries/bin/version)
vlabbuild=$(../.binaries/bin/version -b)
echo "Vlab version is "${vlabversion}
vlabname="vlab-$vlabversion"
rootdir="$vlabname"
tmpdir="/tmp/vlab-$vlabversion-tmpdir"
rmbin="/bin/rm"
cpbin="/bin/cp"
symlinkbin="/bin/ln -s"
tarbin="/bin/tar"
gzipbin="gzip"
gunzipbin="gunzip"
oofsdir=$rootdir/oofs


# make sure we are in the right directory
if [ ! -f makedistr-linux.sh ]; then
  echo "You are in the wrong directory. Please 'cd' to the directory where"
  echo "this script is located."
  exit 1
fi

# test utilities
#echo -n 'sanity check...'
#for prog in $rmbin $cpbin $tarbin $gzipbin ; do
#    if [ -x $prog ] ; then
#	echo -n '.'
#    else
#	echo "FAILED - cannot find executable $prog"
#	exit 1
#    fi
#done
#echo "OK"


# prepare tmpdir
if [ -d $tmpdir ] ; then
    echo -n "deleting temporary directory $tmpdir..."
    if $rmbin -rf $tmpdir ; then
	echo "OK"
    else
	echo "FAILED rm -rf $tmpdir"
	exit 1
    fi
fi
echo -n "making temporary directory $tmpdir..."
if mkdir -p $tmpdir ; then
    echo "OK"
else
    echo "FAILED mkdir -p $tmpdir"
    exit 1
fi

# delete the old distribution
if [ -d $rootdir ] ; then
  echo -n "deleting old distribution..."
  if $rmbin -rf $rootdir ; then
    echo "OK"
  else
    echo "FAILED"
    exit 1
  fi
fi

# make the directory structure
echo -n "building basic directory structure..."
directories="$rootdir $rootdir/oofs \
$rootdir/include $rootdir/config $rootdir/config/vimfiles $rootdir/bin $rootdir/bin/ENVIRO $rootdir/share $rootdir/lib $oofsdir"
for dir in $directories ; do
  if /bin/mkdir -p $dir ; then
    echo -n '.'
  else
    echo "FAILED"
    exit 1
  fi
done
echo "OK"

# copy all scripts
echo -n "copying scripts..."
scripts="postinstall.sh setup-tail.sh sourceme-tail.sh sourceme-tail.csh govlab.sh"
for script in $scripts ; do
    if $cpbin ../config/scripts/$script $rootdir/bin/. ; then
	echo -n '.'
    else
	echo "FAILED to copy $script"
	exit 1
    fi
done
echo "OK"

# copy the readme file
echo -n "copying readme file..."
$cpbin README.linux "$rootdir/README"
echo "OK"

# copy all config files
echo -n "copying all config files..."
config_files="browser rapasswords"
for cfg in $config_files; do
  if $cpbin ../config/default_configs/$cfg $rootdir/config/. ; then
    echo -n '.'
  else
    echo 'FAILED on $cfg'
    exit 1;
  fi
done
config_files_os="object"
for cfg in $config_files_os; do
  if $cpbin ../config/default_configs/Linux/$cfg $rootdir/config/. ; then
    echo -n '.'
  else
    echo 'FAILED on $cfg'
    exit 1;
  fi
done
if $cpbin ../lpfg/scripts/lpfg.mak.Linux $rootdir/config/lpfg.mak ; then
  echo -n '.'
else
  echo 'FAILED on lpfg.mak'
  exit 1;
fi
vim_files="cpfg.vim lpfg.vim lsyntax.vim"
for cfg in $vim_files; do
  if $cpbin ../config/vimfiles/$cfg $rootdir/config/vimfiles/. ; then
    echo -n '.'
  else
    echo 'FAILED on $cfg'
    exit 1;
  fi
done
echo "OK"

# figure out the full name of this platform and store it in .platform file
platform=`/bin/sh ../config/config.guess`
echo "Platform = $platform"
echo "$platform" > "$rootdir/.platform"

# copy all executables
bindir="$rootdir/bin"
echo -n "copying executables..."
#programs="\
#awkped/awkped \
#bezieredit/bezieredit \
#browser/browser \
#cpfg/cpfg vlabcpp/vlabcpp \
#snapicon/snapicon \
#gallery/gallery \
#cuspy/cuspy \
#funcedit/funcedit \
#medit/medit \
#object/object \
#palette/palette \
#panels/panel \
#ped/ped \
#PreProc/preproc \
#raserver/raserver \
#vlabd/vlabd \
#vlab-splash/vlab-splash \
#lpfg/lpfg lpfg/scripts/preproc.sh lpfg/scripts/cmpl.sh \
#l2c/l2c \
#stedit/stedit \
#Rayshade/rayshade/rayshade \
#Rayshade/raypaint/raypaint \
#vlabTextEditor/vlabTextEdit \
#.binaries/bin \
#"
programs="
awkped/awkped \
lpfg/scripts/preproc.sh lpfg/scripts/cmpl.sh \
.binaries/bin \
"
for prg in $programs ; do
  #echo $prg
  if [ -d ../$prg ]; then
    for exe in ../$prg/*; do
      if $cpbin "$exe" "$bindir/."; then
        echo -n '.'
      else
        echo "FAILED on $prg/$exe"
        exit 1
      fi
    done
  else
    if $cpbin "../$prg" "$bindir/."; then
      echo -n '.'
    else
      echo "FAILED on $prg"
      exit 1
    fi
  fi
done
# these scripts are not by default executable, so...
chmod +x $bindir/preproc.sh $bindir/cmpl.sh 

# environmental programs
# the loop above, copies all environmental programs into $bindir/
#envprograms="\
#arvo \
#clover \
#density3d \
#multiple \
#soil \
#chiba \
#density \
#ecosystem \
#honda81 \
#MonteCarlo \
#radiosity \
#takenaka \
#ornament \
#"
#for eprg in $envprograms; do
#  if $cpbin "../.binaries/bin/$eprg" "$bindir/ENVIRO"; then
#    echo -n '.'
#  else
#    echo "FAILED on $eprg"
#    exit 1
#  fi
#done

if $symlinkbin ecosystem "$bindir/ENVIRO/ecosystemR"; then
    echo -n '.'
else
    echo "FAILED to link ecosystem to ecosystemR"
    exit 1
fi

echo "OK"

# copy include files
includedir="$rootdir/include"
echo -n "copying include files..."
includes="lintrfc.h  lparams.h  lpfgall.h  lsys.h  stdmods.h"
for inc in $includes ; do
  if $cpbin "../lpfg/include/$inc" "$includedir/."; then
    echo -n '.'
  else
    echo "FAILED on $inc"
    exit 1
  fi
done
for inc in ../.binaries/include/*; do
  if $cpbin -R "$inc" "$includedir/."; then
    echo -n '.'
  else
    echo "FALED on $inc"
    exit 1
  fi
done
echo "OK"

# Copy libraries
libdir="$rootdir/lib"
echo -n "copying libraries"
#for l in ../.binaries/lib/*; do
for l in ../.libraries/*; do
  if $cpbin -R "$l" "$libdir/."; then
    echo -n '.'
  else
    echo "FAILED on $l"
  fi
done
echo "OK"

# Copy resources
sharedir="$rootdir/share"
echo -n "copying resources"
for r in ../.binaries/share/*; do
  if $cpbin -R "$r" "$sharedir/."; then
    echo -n '.'
  else
    echo "FAILED on $r"
  fi
done
echo "OK"

# copy documentation:
echo -n "copying documentation..."
cp -rf ../.binaries/docs $rootdir/
echo "OK"

# copy oofs
# generates message:
# tar: Ignoring unknown extended header keyword 'LIBARCHIVE.xattr.com.apple.macl'
# macOS uses BSD tar and creates extra info that is not recognized by GNU tar.
# But it is OK to ignore it on Linux-based systems
echo -n "copying oofs..."
#$gunzipbin < ../data/OOFS.tgz | ( cd $oofsdir ; $tarbin xf - )
cp -rf ../oofs $rootdir
echo "OK"

# clean up
echo -n "cleaning up distribution directory..."
# remove .svn files
$rmbin -rf `find $rootdir -type d -name ".svn"`
$rmbin -rf `find $rootdir -type d -name ".git"`
echo "OK"

# create a tarfile
tarname="$vlabname-$vlabbuild-$platform.tar.gz"
echo -n "creating tarfile $tarname..."
# get rid of the old tar file
if [ -f $tarname ]; then
    if $rmbin -f $tarname ; then
	echo -n '(deleted old)...'
    else
	echo "FAILED - to remove the old $tarname"
	exit 1
    fi
fi
if ($tarbin cf - $rootdir | $gzipbin > $tarname) ; then
    echo 'OK'
else
    echo 'FAILED - tar or gzip'
    exit 1
fi

# remove the temporary files
if [ -d $tmpdir ] ; then
    echo -n "deleting temporary directory $tmpdir..."
    if $rmbin -rf $tmpdir ; then
        echo "OK"
    else
        echo "FAILED $rmbin -rf $tmpdir"
        exit 1
    fi
fi
