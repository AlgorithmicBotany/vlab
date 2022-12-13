vlabversion="5.0"
vlabname="vlab-$vlabversion"
rootdir="$vlabname"
tmpdir="/tmp/vlab-$vlabversion-tmpdir"
rmbin="/bin/rm"
cpbin="/bin/cp"
tarbin="/bin/tar"
gzipbin="gzip"
mkdirbin="/bin/mkdir"
lsbin="/bin/ls"
lnbin="/bin/ln"
catbin="/bin/cat"
chmodbin="/bin/chmod"

# test utilities
echo -n 'sanity check...'
for prog in $rmbin $cpbin $tarbin $mkdirbin $lsbin $lnbin $catbin $chmod ; do
    if [ -x $prog ] ; then
	echo -n '.'
    else
	echo "FAILED - cannot find executable $prog"
	exit 1
    fi
done
echo "OK"

echo '**************************'
echo '*                        *'
echo '* Welcome to VLAB setup. *'
echo '*                        *'
echo '**************************'
echo

# make sure we have the VLAB directory
if cd $INSTDIR ; then
    echo "# chdir ok"
else
    echo
    echo "ERROR: cannot chdir to $INSTDIR"
    echo
    exit 1
fi
if [ -d bin -a -d config -a -d oofs -a -d docs ] ; then
    echo ''
else
    echo
    echo 'ERROR: This does not appear to be VLAB directory.'
    echo
    exit 1
fi

# determine vlab installation directory
INSTDIR2=`cat .instdir`
if [ $INSTDIR != $INSTDIR2 ] ; then
    echo
    echo "ERROR: postinstall.sh was not run in this directory. Aborting."
    echo
    exit 1
fi
echo "# VLAB is installed in $INSTDIR"

VLABROOT="$HOME/vlab-4.0"
echo "The usual place to install VLAB setup files is in your home directory."
echo "I think in your case this is:"
echo
echo "             $VLABROOT"
echo
echo "If you want to accept this choice, press ENTER. If you wish to install"
echo "somwhere else, type in the location."
echo -n '--> '
read dst
if [ "x$dst" != "x" ]; then
    VLABROOT=$dst
fi

echo
echo "You have decided to install VLAB files in:"
echo
echo "             $VLABROOT"
echo
echo -n 'Is this correct [y/n]? '
read ans
if [ "$ans" != "y" -a "$ans" != "yes" -a "$ans" != "Y" -a "$ans" != "YES" ] ; then
    echo
    echo "******  VLAB setup ABORTED *************"
    echo
    exit 1
fi

# make sure the directory doesn't already exist
if [ -d $VLABROOT ] ; then
    echo
    echo "ERROR: directory already exists. If you are sure you do not want"
    echo "       this directory, delete it and run this program again."
    echo
    exit 1
fi

# create the directory
if $mkdirbin -p $VLABROOT ; then
    echo "# directory created"
else
    echo
    echo "ERROR: Could not create such directory. Make sure there is enough"
    echo "       room on the dist space and that you have write access"
    echo "       to the directory you selected."
    echo
    exit 1
fi

# make a symbolic links for pdf-docs
if ln -s $INSTDIR/docs $VLABROOT/docs ; then
    echo "# symbolic link created for docs"
else
    echo
    echo "ERROR: could not create sym-link for docs"
    echo
    exit 1
fi


# make symbolic links for include files
if $mkdirbin -p "$VLABROOT/include" ; then
    echo "# include dir created"
else
    echo
    echo "ERROR: could not create include directory"
    echo
    exit 1
fi
headers=`$lsbin -1 $INSTDIR/include`
for header in $headers ; do
    if $lnbin -s $INSTDIR/include/$header $VLABROOT/include/$header ; then
	echo "# $header linked"
    else
	echo
	echo "ERROR: Failed to sym-link header file $header."
	echo
	exit 1
    fi
done

# make symbolic links for programs in bin/
if $mkdirbin -p "$VLABROOT/bin" ; then
    echo "# bin dir created"
else
    echo
    echo "ERROR: could not create binary directory."
    echo
    exit 1
fi
progs=`$lsbin -1 $INSTDIR/bin`
for prog in $progs ; do
    if $lnbin -s $INSTDIR/bin/$prog $VLABROOT/bin/$prog ; then
	echo "# $prog linked"
    else
	echo
	echo "ERROR: Failed to sym-link executable $prog."
	echo
	exit 1
    fi
done

# remove symbolic links for shell-scripts
$rmbin `$lsbin $VLABROOT/bin/*.sh`
$rmbin `$lsbin $VLABROOT/bin/*.csh`
# recreate cmpl.sh and preproc.sh
progs=cmpl.sh preproc.sh
for prog in $progs ; do
    if $lnbin -s $INSTDIR/bin/$prog $VLABROOT/bin/$prog ; then
	echo "# $prog linked"
    else
	echo
	echo "ERROR: Failed to sym-link $prog."
	echo
	exit 1
    fi
done

# generate the sourceme.sh script
sourcemesh="$VLABROOT/bin/sourceme.sh"
if echo "#!/bin/sh" > $sourcemesh &&
   echo "VLABROOT=\"$VLABROOT\"" >> $sourcemesh &&
   $catbin $INSTDIR/bin/sourceme-tail.sh >> $sourcemesh ; then
    echo "# sourceme.sh generated"
else
    echo
    echo "ERROR: could not generate sourceme.sh script"
    echo
    exit 1
fi
chmod a+x $sourcemesh

# generate the sourceme.sh script
sourcemecsh="$VLABROOT/bin/sourceme.csh"
if echo "#!/bin/csh" > $sourcemecsh &&
   echo "setenv VLABROOT \"$VLABROOT\"" >> $sourcemecsh &&
   $catbin $INSTDIR/bin/sourceme-tail.csh >> $sourcemecsh ; then
    echo "# sourceme.csh generated"
else
    echo
    echo "ERROR: could not generate sourcme.csh script"
    echo
    exit 1
fi
chmod a+x $sourcemecsh

# copy subdirectories config and oofs
# copy entire directories
dirs="config oofs"
for dir in $dirs ; do
    if $tarbin cf - $dir | ( cd $VLABROOT ; tar xf - ) ; then
	echo "# copied directory $dir"
    else
	echo "ERROR: could not copy directory $dir"
    fi
done

