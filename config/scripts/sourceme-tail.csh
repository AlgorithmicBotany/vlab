setenv VLABBIN "${VLABROOT}/bin"
setenv PATH "${VLABBIN}:${VLABBIN}/ENVIRO:${PATH}"
setenv VLABDOCDIR "$VLABROOT/docs"
setenv VLABOBJECTBIN "$VLABBIN/object"
setenv VLABDAEMONBIN "$VLABBIN/vlabd"
setenv VLABCONFIGDIR "$VLABROOT/config"
setenv VLABHBROWSERBIN "$VLABBIN/hbrowser"
setenv VLABBROWSERBIN "$VLABBIN/browser"
setenv LPFGPATH "$VLABROOT"
setenv LPFGRESOURCES "$VLABROOT"
setenv SIMFRAMEPATH "$VLABROOT"
setenv VVDIR "$VLABROOT"
setenv VVEDIR "$VLABROOT"
setenv LD_LIBRARY_PATH "$VLABROOT/lib:$LD_LIBRARY_PATH"

#this is where vlab will store temporary files - should be a directory
#on a local disk, so that the access is fast

setenv VLABTMPDIR /tmp

#set the directory for the resource files (for Xt)
if( "${?XUSERFILESEARCHPATH}" == "0" ) then
  setenv XUSERFILESEARCHPATH "${VLABROOT}/app-defaults/%N"
else
  setenv XUSERFILESEARCHPATH "${VLABROOT}/app-defaults/%N:${XUSERFILESEARCHPATH}"
endif

