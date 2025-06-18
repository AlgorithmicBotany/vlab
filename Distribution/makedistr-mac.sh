#!/bin/bash

vlabversion=`../.binaries/version.app/Contents/MacOS/version`
echo "Vlab version is "${vlabversion}
vlabbuild=`../.binaries/version.app/Contents/MacOS/version -b`
vlabname="vlab-$vlabversion"
rootdir="$(pwd)/$vlabname"
rmbin="rm"
cpbin="cp"
chmodbin="chmod"
mvbin="mv"
lnbin="ln"
tarbin="tar"
gzipbin="gzip"
gunzipbin="gunzip"
mkdirbin="mkdir"
touchbin="touch"

# make sure we are in the right directory
if [ ! -f makedistr-mac.sh ]; then
  echo "You are in the wrong directory. Please 'cd' to the directory where"
  echo "this script is located."
  exit 1
fi

# figure out the full name of this platform and store it in .platform file
platform=`/bin/sh ../config/config.guess`
echo "Platform = $platform"
# echo "$platform" > "$rootdir/.platform"

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
if $mkdirbin -p $rootdir ; then
  echo -n "."
else
  echo "FAILED"
  exit 1
fi
echo 'OK'

# copy structure of browser.app
echo -n "copying structure of browser.app "
if $cpbin -Rp ../.binaries/browser.app $rootdir ; then
  echo -n '.'
else
  echo "FAILED: $cpbin -Rp ../.binaries/browser.app $rootdir"
  exit 1
fi
echo "OK"

# mark the root directory
echo -n "Marking root directory..."
$touchbin $rootdir/browser.app/TopLevelMarker
echo 'OK'

browserdir="$rootdir/browser.app/Contents"
plugindir=$browserdir/Plug-ins
systemdir=$browserdir/System
resourcedir=$browserdir/Resources
oofsdir=$resourcedir/oofs
vlabbindir=$browserdir/MacOS/bin
vlabdbindir=$browserdir/MacOS/dbin

echo -n "creating bundle structure for browser.app "

directories="$browserdir $plugindir $systemdir $resourcedir $resourcedir/config $resourcedir/English.lproj $vlabbindir $vlabdbindir $oofsdir"

for dir in $directories ; do
  #echo -n "   $dir "
  if $mkdirbin -p $dir ; then
    echo -n "."
  else
    echo "FAILED"
    exit 1
  fi
done
echo "OK"

# copy all config files
echo -n "copying all config files..."
config_files=("rapasswords")
for cfg in ${config_files[*]}; do
  if $cpbin -p ../config/default_configs/$cfg $resourcedir/config/. ; then
    echo -n '.'
  else
    echo 'FAILED'
    exit 1;
  fi
done
config_files_os="object"
for cfg in $config_files_os; do
  if $cpbin -p ../config/default_configs/OSX/$cfg $resourcedir/config/. ; then
    echo -n '.'
  else
    echo 'FAILED on $cfg'
    exit 1;
  fi
done
echo "OK"

# copy oofs
echo -n "copying oofs..."
#$gunzipbin < ../data/oofs.tar.gz | ( cd $resourcedir ; $tarbin xf - )
#$gunzipbin < ../data/OOFS.tgz | ( cd $oofsdir ; $tarbin xf - )
$cpbin -Rp ../oofs $resourcedir
echo "done"

# copy oofs
echo -n "copying Gifts ..."
#echo "$cpbin -r ./Gifts $oofsdir/../"
#pwd
$cpbin -r ./Gifts $oofsdir/../
echo "done"


system_programs="object.app version.app vlab-splash.app vlabd.app"
plugin_programs="MonteCarlo.app QuasiMC.app arvo.app bezieredit.app stedit.app 
chiba.app clover.app ornament.app cpfg.app vlabcpp.app cuspy.app density.app density3d.app ecosystem.app funcedit.app gallery.app timeline.app
honda81.app l2c.app lpfg.app medit.app multiple.app palette.app  
panel.app ped.app preproc.app radiosity.app snapicon.app soil.app shadowpyramid.app takenaka.app vlabTextEdit.app rayshade.app raypaint.app
vv2cpp.app vvinterpreter.app"
plugin_librairies="libcomm.a ../libs/comm/message.c ../libs/comm/comm_lib.h"

# copy system programs
echo -n "copying system programs..."
for prg in $system_programs; do
  if $cpbin -Rp ../.binaries/$prg $systemdir/. ; then
    echo -n '.'
  else
    echo "FAILED: $cpbin -Rp ../.binaries/$prg $systemdir/."
    exit 1
  fi
done
echo "OK"

# copy libraries
echo -n "copying plugin libraries ..."
for prg in $plugin_librairies; do
  if $cpbin -Rp ../.libraries/$prg $plugindir/. ; then
    echo -n '.'
  else
    echo "FAILED: $cpbin -Rp ../.libraries/$prg $plugindir/."
    exit 1
  fi
done
echo "OK"


# copy plug-ins
echo -n "copying plug-ins..."
for prg in $plugin_programs; do
  if $cpbin -Rp ../.binaries/$prg $plugindir/. ; then
    echo -n '.'
  else
    echo "FAILED: $cpbin -Rp ../.binaries/$prg $plugindir/."
    #exit 1
  fi
done
echo "OK"

# Creating symbolic link for environment programming
echo -n 'Creating symbolic link for environment programming:'
(cd $plugindir ; ln -s ecosystem ecosystemR )
echo " OK"

# Copying awkped
echo -n "copying awkped: "
if $cpbin ../awkped/awkped $plugindir ; then
  echo -n '.'
else
  echo " FAILED"
  exit 1
fi
echo " OK"

# setting up the Info.plist
keys[0]="CFBundleShortVersionString"
values[0]="$vlabversion"

echo -n "Setting up Info.plist for browser.app "

for ((i=0; i < ${#keys[@]}; i++)); do
  if defaults write $browserdir/Info ${keys[$i]} "${values[$i]}" ; then
    echo -n '.'
  else
    echo "FAILED: defaults write $browserdir/Info ${keys[$i]} \"${values[$i]}\""
    exit 1
  fi
done
echo 'OK'

# copy raserver
echo -n "copying raserver"
if $cpbin -Rp ../.binaries/raserver.app $rootdir ; then
  echo -n '.'
else
  echo "FAILED: $cpbin -Rp ../.binaries/raserver.app $rootdir"
  exit 1
fi
echo "OK"

# move raserver configuration files
echo -n "copying raserver configuation files"
raresources="$rootdir/raserver.app/Contents/Resources"
if $mvbin $resourcedir/config/rapasswords $raresources ; then
  echo -n '.'
else
  echo "FAILED: $mvbin $resourcedir/config/rapasswords $raresources"
  exit 1
fi
echo "OK"

# populate the bin directory
echo "Populating bin directory:"
echo "You can put your own binaries here if you wish." > $vlabbindir/README
echo "WARNING WARNING WARNING WARNING WARNING WARNING WARNING" >> $vlabdbindir/README
echo "-------------------------------------------------------" >> $vlabdbindir/README
echo "Please do not put binaries in this dbin directory." >> $vlabdbindir/README
echo "They will be deleted by the automatic update process." >> $vlabdbindir/README
if $cpbin ../.binaries/updatebin $vlabbindir/updatebin ; then
    echo "  temporary updatebin copied"
else
    echo "failed to make temporary copy updatebin"
    exit 1
fi
echo "  scripts for programs"
$vlabbindir/updatebin
#updatebin is no longer needed...
echo $vlabbindir/updatebin
$rmbin -f $vlabbindir/updatebin
echo "  temporary updatebin removed"
echo "OK"

#  # copy all scripts
  echo -n "copying lpfg scripts..."
  scripts="cmpl.sh preproc.sh "
  for script in $scripts ; do
      if $cpbin ../.binaries/lpfg.app/Contents/MacOs/$script $vlabbindir/. ; then
	$chmodbin 755 $vlabbindir/$script
  	echo -n '.'
      else
  	echo "FAILED to copy $script"
  	exit 1
      fi
      if $cpbin ../.binaries/lpfg.app/Contents/MacOs/$script $vlabdbindir/. ; then
	$chmodbin 755 $vlabdbindir/$script
  	echo -n '.'
      else
  	echo "FAILED to copy $script"
  	exit 1
      fi
  done
  echo "OK"

# clean up
echo -n "cleaning up distribution directory..."
# remove .svn files
$rmbin -rf `find $rootdir -type d -name ".svn"`
echo "done"

