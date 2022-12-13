# Input file
FILE=$1
DESTFILE=$2
# How many seconds before file is deemed "older"
OLDTIME=120
# Get current and file times
CURTIME=$(date +%s)
FILETIME=$(stat -f "%m" $FILE)
TIMEDIFF=$(expr $CURTIME - $FILETIME)

# Check if file older
#if [ $TIMEDIFF -lt $OLDTIME ]; then
#     cp $1 $2
#    
#fi

diffFiles=`diff $FILE $DESTFILE`
if [[ $(diff $FILE $DESTFILE) ]]
then
    cp $1 $2
fi
