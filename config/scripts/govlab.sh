#!/bin/sh

# make sure we are in the VLAB directory
if ! [ -d bin -a -d config -a -d oofs -a -d docs ] ; then
    echo
    echo 'ERROR: You must execute this script from the root VLAB directory: ./bin/govlab.sh'
    echo
    exit 1
fi

# if the sourceme.sh file does not exist, generate it
sourcefile=./bin/sourceme.sh
if [ ! -f "$sourcefile" ]; then
	./bin/postinstall.sh > /dev/null 2>&1
fi
	
# if that did not work, report it
if [ ! -f "$sourcefile" ]; then
	echo "Post installation script did not generate a source file."
	echo "Try doing it manually. See README file."
	exit 1
fi

# now, source the file if necessary
if [ -z "$VLABROOT" ]; then
	set -a
	. "$sourcefile"
	set +a
fi

# call browser
./bin/browser &

