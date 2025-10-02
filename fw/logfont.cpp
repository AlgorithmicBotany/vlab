/**************************************************************************

  File:		logfont.cpp
  Created:	08-Jan-98


  Implementation of class LogFont


**************************************************************************/

#include <assert.h>

#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "logfont.h"


LogFont::LogFont(const LOGFONT& lf)
{
	lfHeight = lf.lfHeight;
	lfWidth = lf.lfWidth;
	lfEscapement = lf.lfEscapement;
	lfOrientation = lf.lfOrientation;
	lfWeight = lf.lfWeight;
	lfItalic = lf.lfItalic;
	lfUnderline = lf.lfUnderline;
	lfStrikeOut = lf.lfStrikeOut;
	lfCharSet = lf.lfCharSet;
	lfOutPrecision = lf.lfOutPrecision;
	lfClipPrecision = lf.lfClipPrecision;
	lfQuality = lf.lfQuality;
	lfPitchAndFamily = lf.lfPitchAndFamily;
	_tcscpy(lfFaceName, lf.lfFaceName);
}


LogFont::LogFont(int height, const std::string& facename)
{
	lfHeight = height;
	lfWidth = 0;
	lfEscapement = 0;
	lfOrientation = 0;
	lfWeight = FW_DONTCARE;
	lfItalic = false;
	lfUnderline = false;
	lfStrikeOut = false;
	lfCharSet = DEFAULT_CHARSET;
	lfOutPrecision = OUT_DEFAULT_PRECIS;
	lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lfQuality = DEFAULT_QUALITY;
	lfPitchAndFamily = DEFAULT_PITCH;
	_tcscpy(lfFaceName, facename.c_str());
}
