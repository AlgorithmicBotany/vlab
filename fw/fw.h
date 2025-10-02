/**************************************************************************

  File:		fw.h
  Created:	25-Nov-97


  General include file for Framework


**************************************************************************/


#ifndef __FW_H__
#define __FW_H__

#ifndef STRICT
	#error STRICT not defined
#endif

#ifdef assert
	#error Do not include assert.h file
#endif

#include <cassert>
#include <string>

#ifdef _WINDOWS_
	#error Do not include windows.h file
#endif

#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <objbase.h>
#include <olectl.h>
#include <windowsx.h>

#include <cstdio>


#ifdef _INC_TCHAR
	#error Do not include tchar.h file
#endif


#include <tchar.h>

#include "warningset.h"

#include "fwdefs.h"
#include "scrnpnt.h"
#include "wndclass.h"
#include "logfont.h"
#include "comdlgclss.h"
#include "mdimenus.h"
#include "exception.h"
#include "menu.h"
#include "window.h"
#include "app.h"
#include "editline.h"
#include "sizestate.h"
#include "statusbar.h"
#include "combobox.h"
#include "listbox.h"
#include "tabctrl.h"
#include "trackbarctrl.h"
#include "spinctrl.h"
#include "button.h"
#include "gdiobjs.h"
#include "keystate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "scrollctrl.h"
#include "formctrl.h"
#include "mdiclient.h"
#include "appctrl.h"
#include "mdictrl.h"
#include "mdichctrl.h"
#include "registry.h"
#include "lstring.h"
#include "thread.h"
#include "dialog.h"
#include "canvas.h"
#include "bitmap.h"
#include "file.h"
#include "stack.h"
#include "semaphore.h"
#include "mutex.h"
#include "dropfiles.h"
#include "timer.h"
#include "fwtmplts.h"
#include "tmpfile.h"
#include "tmpdir.h"
#include "findfile.h"
#include "newprocess.h"
#include "clipboard.h"
#include "gconstrain.h"
#include "owndrwbtn.h"
#include "imagelist.h"
#include "strbuf.h"
#include "comtmplts.h"
#include "resstrng.h"
#include "folderbrowser.h"
#include "colorslider.h"
#include "colors.h"
#include "pickclrdlg.h"
#include "tooltip.h"
#include "listview.h"
#include "log.h"
#include "winmaker.h"
#include "rawmemory.h"
#include "subclass.h"
#include "childenum.h"
#include "progressbar.h"
#include "dynlib.h"
#include "genctrl.h"
#include "fwuser.h"
#include "cmndline.h"
#include "configfile.h"
#include "resrc.h"
#include "prcss.h"
#include "flagset.h"
#include "ShellShortcut.h"

#endif
