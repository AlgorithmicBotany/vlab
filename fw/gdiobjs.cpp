/**************************************************************************

  File:		gdiobjs.cpp
  Created:	13-Jan-98


  Implementation of GDIObjects classes 


**************************************************************************/


#include <cassert>

#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "gdiobjs.h"
#include "exception.h"
#include "logfont.h"
#include "libstrng.h"


Font::Font(LONG height, const std::string& fname)
{
	LOGFONT lf;
	{
		lf.lfHeight = height;
		lf.lfWidth = 0;
		lf.lfEscapement = 0;
		lf.lfOrientation = 0;
		lf.lfWeight = 0;
		lf.lfItalic = FALSE;
		lf.lfUnderline = FALSE;
		lf.lfStrikeOut = FALSE;
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = DEFAULT_QUALITY;
		lf.lfPitchAndFamily = 0;
		_tcscpy(lf.lfFaceName, fname.c_str());
	}
	SetObj(CreateFontIndirect(&lf));
	if (0 == GetObj())
		throw Exception(0, FWStr::CreateFont);
}


Font::Font(const LogFont& lf)
{
	SetObj(CreateFontIndirect(&lf));
	if (0 == GetObj())
		throw Exception(0, FWStr::CreateFont);
}


void RegionBase::Combine(const RegionBase& r1, const RegionBase& r2, RegionBase::CmbOp op)
{
	CombineRgn(_hRgn, r1.Handle(), r2.Handle(), op);
}

Region::Region(int l, int t, int r, int b)
{
	Set(CreateRectRgn(l, t, r, b));
}


Region::Region(const RegionBase& r1, const RegionBase& r2, CmbOp op)
{
	Set(CreateRectRgn(0, 0, 0, 0));
	Combine(r1, r2, op);
}


CircleRegion::CircleRegion(int left, int top, int right, int bottom)
{
	Set(CreateEllipticRgn(left, top, right, bottom));
}


void ExchangeGDIobjects(GDIObject& a, GDIObject& b)
{
	HGDIOBJ hTmp = a._hObj;
	a._hObj = b._hObj;
	b._hObj = hTmp;
}


Pens3d::Pens3d () : 
_penLight(GetSysColor (COLOR_3DLIGHT)),
_penHilight(GetSysColor (COLOR_3DHILIGHT)),
_penFace(GetSysColor(COLOR_3DFACE)),
_penShadow(GetSysColor (COLOR_3DSHADOW)),
_penDkShadow(GetSysColor (COLOR_3DDKSHADOW))
{}


Pens3d::~Pens3d()
{}

Pens3d pens3Dset;
