/**************************************************************************

  File:		comdlgclss.cpp
  Created:	08-Jan-98


  Implementation of common dialogs classes


**************************************************************************/


#include <assert.h>

#include <cstdlib>

#include <string>

#include <tchar.h>
#include <windows.h>
#include <commdlg.h>

#include "warningset.h"

#include "logfont.h"
#include "comdlgclss.h"
#include "mdimenus.h"
#include "exception.h"
#include "menu.h"
#include "window.h"
#include "app.h"



Choosefont::Choosefont(HWND hwnd, int height, const std::string& fn) : _lf(height, fn)
{
	lStructSize = sizeof(CHOOSEFONT);
	hwndOwner = hwnd;
	hDC = 0;
	lpLogFont = &_lf;
	iPointSize = 0;
	Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
	rgbColors = 0;
	lCustData = 0;
	lpfnHook = 0;
	lpTemplateName = 0;
	hInstance = 0;
	lpszStyle = 0;
	nFontType = 0;
	nSizeMin = 0;
	nSizeMax = 0;
}

Choosefont::Choosefont(HWND hwnd, const LogFont& lf) : _lf(lf)
{
	lStructSize = sizeof(CHOOSEFONT);
	hwndOwner = hwnd;
	hDC = 0;
	lpLogFont = &_lf;
	iPointSize = 0;
	Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
	rgbColors = 0;
	lCustData = 0;
	lpfnHook = 0;
	lpTemplateName = 0;
	hInstance = 0;
	lpszStyle = 0;
	nFontType = 0;
	nSizeMin = 0;
	nSizeMax = 0;
}


BOOL Choosefont::Display()
{
	return ChooseFont(this);
}



OpenFilename::OpenFilename(HWND hwnd, UINT sid, const std::string& defExt)
{
#ifndef NDEBUG
	int res =
#endif
	LoadString(App::GetInstance(), sid, _Filter, 256);
	assert(res>0);

	_tcsncpy(_DefExt, defExt.c_str(), nMaxExtSize);
	_DefExt[nMaxExtSize] = 0;

	// Change tabs to nulls

	TCHAR* tochange = _tcschr(_Filter, __TEXT('\t'));
	while (0 != tochange)
	{
		*tochange = 0;
		tochange++;
		tochange = _tcschr(tochange, __TEXT('\t'));
	}

	_Filename[0] = 0;
	_FileTitle[0] = 0;
	_InitDir[0] = 0;
	memset(&lStructSize, 0, sizeof(OPENFILENAME));
	lStructSize = sizeof(OPENFILENAME);
	hwndOwner = hwnd;
	hInstance = 0;
	lpstrFilter = _Filter;
	lpstrCustomFilter = 0;
	nMaxCustFilter = 0;
	nFilterIndex = 1;
	lpstrFile = _Filename;
	nMaxFile = _MAX_PATH+1;
	lpstrFileTitle = _FileTitle;
	nMaxFileTitle = nMaxTitleSize;
	lpstrInitialDir = 0;
	lpstrTitle = 0;

	Flags = OFN_NOCHANGEDIR;
	lpstrDefExt = _DefExt;
	lCustData = 0;
	lpfnHook = 0;
	lpTemplateName = 0;
	pvReserved = NULL;
	dwReserved = 0;
	FlagsEx = 0;
}


void OpenFilename::SetDefault(const std::string& defname)
{
	_tcsncpy(_Filename, defname.c_str(), _MAX_PATH);
	_Filename[_MAX_PATH] = 0;
}

void OpenFilename::SetDirectory(const std::string& dir)
{
	strncpy(_InitDir, dir.c_str(), _MAX_PATH);
	_InitDir[_MAX_PATH] = 0;
	lpstrInitialDir = _InitDir;
}



Choosecolor::Choosecolor(HWND hWnd, COLORREF init)
{
	lStructSize = sizeof(CHOOSECOLOR);
	hwndOwner = hWnd;
	hInstance = 0;
	rgbResult = init;
	lpCustColors = _CustColors;
	Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
	lCustData = 0;
	lpfnHook = 0;
	lpTemplateName = 0;
}


BOOL Choosecolor::Choose()
{
	return ChooseColor(this);
}
