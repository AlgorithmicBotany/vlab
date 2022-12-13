export VLABBIN="${VLABROOT}/bin"
export PATH="${VLABBIN}:${VLABBIN}/ENVIRO:${PATH}"
export VLABDOCDIR="${VLABROOT}/docs"
export VLABOBJECTBIN="${VLABBIN}/object"
export VLABDAEMONBIN="${VLABBIN}/vlabd"
export VLABCONFIGDIR="${VLABROOT}/config"
export VLABHBROWSERBIN="${VLABBIN}/hbrowser"
export VLABBROWSERBIN="${VLABBIN}/browser"
export LPFGPATH="${VLABROOT}"
export LPFGRESOURCES="${VLABROOT}"
export VVDIR="$VLABROOT"
export VVEDIR="$VLABROOT"
export LD_LIBRARY_PATH="$VLABROOT/lib:$LD_LIBRARY_PATH"

#this is where vlab will store temporary files - should be a directory
#on a local disk, so that the access is fast

export VLABTMPDIR="/tmp"

#set the directory for the resource files (for Xt)
#if [ "x${XUSERFILESEARCHPATH}" = "x" ]; then
#  XUSERFILESEARCHPATH="${VLABROOT}/app-defaults/%N"
#else
#  XUSERFILESEARCHPATH="${VLABROOT}/app-defaults/%N:${XUSERFILESEARCHPATH}"
#fi

