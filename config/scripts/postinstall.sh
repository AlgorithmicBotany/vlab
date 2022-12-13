#!/bin/sh

# make sure we are in the VLAB directory
if [ -d bin -a -d config -a -d oofs -a -d docs ] ; then
    echo 'ok'
else
    echo
    echo 'ERROR: You must execute this script from the root VLAB directory.'
    echo
    exit 1
fi

# determine the current working directory
currdir=$PWD
echo "VLAB is installed in:"
echo
echo "               $currdir"
echo

# save this information to a file
if echo "$currdir" > .instdir ; then
    echo -n ''
else
    echo "ERROR: Could not create file .instdir"
    exit 1
fi

# generate a default sourceme.sh script
sourcemesh="bin/sourceme.sh"
if echo "#!/bin/sh" > $sourcemesh &&
   echo "export VLABROOT=\"$currdir\"" >> $sourcemesh &&
   cat bin/sourceme-tail.sh >> $sourcemesh &&
   chmod a+x $sourcemesh ; then
    echo "# sourceme.sh generated"
else
    echo
    echo "ERROR: could not generate sourcme.sh script"
    echo
    exit 1
fi

# generate the sourceme.csh script
sourcemecsh="bin/sourceme.csh"
if echo "#!/bin/csh" > $sourcemecsh &&
   echo "setenv VLABROOT \"$currdir\"" >> $sourcemecsh &&
   cat bin/sourceme-tail.csh >> $sourcemecsh &&
   chmod a+x $sourcemecsh ; then
    echo "# sourceme.csh generated"
else
    echo
    echo "ERROR: could not generate sourcme.csh script"
    echo
    exit 1
fi

# generate the default setup.sh script
setupsh="bin/setup.sh"
if echo "#!/bin/sh" > $setupsh &&
   echo "INSTDIR=\"$currdir\"" >> $setupsh &&
   cat bin/setup-tail.sh >> $setupsh &&
   chmod a+x $setupsh ; then
    echo "# setup.sh generated"
else
    echo
    echo "ERROR: could not generate setup.sh script"
    echo
    exit 1
fi

echo
echo
echo 'Post-installation done.'
echo
echo 'If you intend to have a multiple-user installation,'
echo 'each user should run setup.sh from the bin directory.'
echo

