/**************************************************************************

  File:		gdiobjs.h
  Created:	13-Jan-98


  Declaration of GDIObjects classes 


**************************************************************************/


#ifndef __GDIOBJECTS_H__
#define __GDIOBJECTS_H__

#include <string>

class GDIObject
{
friend void ExchangeGDIobjects(GDIObject&, GDIObject&);
protected:
	GDIObject() : _hObj(0) {}
public:
	GDIObject(HGDIOBJ hObj) : _hObj(hObj) {}
	~GDIObject() { DeleteObject(_hObj); }
	operator HGDIOBJ() const
	{ return _hObj; }
	HGDIOBJ HObj() const
	{ return _hObj; }
	void Set(HGDIOBJ hObj)
	{
		assert(0 == _hObj);
		_hObj = hObj;
	}
protected:
	void SetObj(HGDIOBJ hObj)
	{ _hObj = hObj; }
	HGDIOBJ GetObj() const
	{ return _hObj; }
private:
	HGDIOBJ _hObj;
};



class Brush : public GDIObject
{
public:
	Brush(COLORREF clr)
	{
		LOGBRUSH lb;
		lb.lbStyle = BS_SOLID;
		lb.lbColor = clr;
		lb.lbHatch = 0;
		SetObj(CreateBrushIndirect(&lb));
	}
	operator HBRUSH() const
	{ return reinterpret_cast<HBRUSH>(GetObj()); }
};


class Pen : public GDIObject
{
public:
	enum penstyle
	{
		psSolid = PS_SOLID,
		psDash = PS_DASH,
		psDot = PS_DOT,
		psDashDot = PS_DASHDOT,
		psDashDotDot = PS_DASHDOTDOT,
		psNull = PS_NULL,
		psInsideFrame = PS_INSIDEFRAME
	};
	Pen(COLORREF clr)
	{
		SetObj(CreatePen(psSolid, 0, clr));
	}
	Pen(COLORREF clr, int width)
	{
		SetObj(CreatePen(psSolid, width, clr));
	}
	Pen(COLORREF clr, penstyle style)
	{
		SetObj(CreatePen(style, 0, clr));
	}

};

class LogFont;

class Font : public GDIObject
{
public:
	Font(HFONT hFont) : GDIObject(hFont) {}
	Font(LONG, const std::string&);
	Font(const LogFont&);
};

class RegionBase
{
public:
	enum CmbOp
	{
		eoAnd = RGN_AND,
		eoCopy = RGN_COPY,
		eoDiff = RGN_DIFF,
		eoOr = RGN_OR,
		eoXor = RGN_XOR
	};
protected:
	RegionBase()
	{ _hRgn = 0; }
	void Set(HRGN hRgn)
	{
		assert(!IsSet());
		_hRgn = hRgn;
	}
	void Combine(const RegionBase&, const RegionBase&, CmbOp);
public:
	~RegionBase()
	{ 
		if (0 != _hRgn)
			DeleteObject(_hRgn); 
	}
	HRGN Handle() const
	{ return _hRgn; }
	HRGN Release() 
	{
		HRGN hRes = _hRgn;
		_hRgn = 0;
		return hRes;
	}
	bool IsSet() const
	{ return 0 != _hRgn; }
	void Swap(RegionBase& r)
	{
		HRGN hTmp = r._hRgn;
		r._hRgn = _hRgn;
		_hRgn = hTmp;
	}
private:
	HRGN _hRgn;
};

class Region : public RegionBase
{
public:
	Region() {}
	Region(int, int, int, int);
	Region(const RegionBase&, const RegionBase&, CmbOp);
};


class CircleRegion : public RegionBase
{
public:
	CircleRegion(int, int, int, int);
};

class Cursor
{
public:
	enum dc
	{
		Arrow = (int)IDC_ARROW,
		Cross = (int)IDC_CROSS,
		AppStarting = (int)IDC_APPSTARTING
	};
	Cursor(const char*);
	Cursor(dc);
	~Cursor();
	operator HCURSOR() const
	{ return _hCursor; }
private:
	bool _shared;
	HCURSOR _hCursor;
};

void ExchangeGDIobjects(GDIObject& a, GDIObject& b);


class Pens3d
{
public:
	Pens3d();
	~Pens3d();
	Pen& Hilight() { return _penHilight; }
	Pen& Light() { return _penLight; }
	Pen& Face() { return _penFace; }
	Pen& Shadow() { return _penShadow; }
	Pen& DkShadow() { return _penDkShadow; }
private:
	Pen		_penHilight;
	Pen		_penLight;
	Pen		_penFace;
	Pen		_penShadow;
	Pen		_penDkShadow;
};

extern Pens3d pens3Dset;


class ROp
{
public:
	enum Rop
	{
		Black = R2_BLACK,
		CopyPen = R2_COPYPEN,
		MaskNotPen = R2_MASKNOTPEN,
		MaskPen = R2_MASKPEN,
		MaskePenNot = R2_MASKPENNOT,
		MergeNotPen = R2_MERGENOTPEN,
		MergePen = R2_MERGEPEN,
		MergePenNot = R2_MERGEPENNOT,
		Nop = R2_NOP,
		Not = R2_NOT,
		NotCopyPen = R2_NOTCOPYPEN,
		NotMaskPen = R2_NOTMASKPEN,
		NotMergePen = R2_NOTMERGEPEN,
		NotXorPen = R2_NOTXORPEN,
		White = R2_WHITE,
		XorPen = R2_XORPEN
	};
	ROp(HDC hdc, Rop rop) : _hdc(hdc)
	{
		_oldrop = SetROP2(_hdc, rop);
	}
	~ROp()
	{
		SetROP2(_hdc, _oldrop);
	}
private:
	HDC _hdc;
	int _oldrop;
};


class TextAlignment
{
public:
	enum VAlign
	{
		BaseLine = TA_BASELINE,
		Bottom = TA_BOTTOM,
		Top = TA_TOP,
		NoUpdateCP = TA_NOUPDATECP,
		RTLReading = TA_RTLREADING,
		UpdateCP = TA_UPDATECP
	};
	enum HAlign
	{
		Center = TA_CENTER,
		Left = TA_LEFT,
		Right = TA_RIGHT
	};
	TextAlignment(HDC hdc, VAlign va) : _hdc(hdc)
	{ 
		_prevAlign = SetTextAlign(_hdc, va);
	}
	TextAlignment(HDC hdc, HAlign ha) : _hdc(hdc)
	{
		_prevAlign = SetTextAlign(_hdc, ha);
	}
	TextAlignment(HDC hdc, VAlign va, HAlign ha) : _hdc(hdc)
	{
		_prevAlign = SetTextAlign(_hdc, va | ha);
	}
	~TextAlignment()
	{
		SetTextAlign(_hdc, _prevAlign);
	}
private:
	HDC _hdc;
	UINT _prevAlign;
};


#endif
