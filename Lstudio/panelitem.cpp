#include <fw.h>

#include "objfgvobject.h"

#include "panelprms.h"
#include "panelitem.h"
#include "panelsldprpdlg.h"
#include "panelbtnprpdlg.h"
#include "panellblprpdlg.h"
#include "panelgrprpdlg.h"
#include "paneldesign.h"

#include "resource.h"


const int GalleryScale = 5;


INIT_COUNTER(PanelItem);


PanelItem::PanelItem(int x, int y, const char* nm)
{
	_origin.x = x;
	_origin.y = y;
	_namelen = strlen(nm);
	assert(_namelen<=PanelParameters::eMaxNameLength);
	strcpy(_name, nm);
	_action[0] = 0;
	_type = PanelParameters::pitUnknown;
	_pDesign = 0;
	INC_COUNTER;
}


void PanelItem::SetAction(const char* bf)
{
	assert(strlen(bf)<=PanelParameters::eMaxActionLength);
	strcpy(_action, bf);
}


void PanelItem::MoveBy(int x, int y)
{
	_origin.x += x;
	_origin.y += y;
	assert(_origin.x>=0);
	assert(_origin.y>=0);
}


void PanelItem::AlignLeft(int l)
{
	RECT r;
	GetRect(r);
	int dst = l - r.left;
	MoveBy(dst, 0);
}


void PanelItem::AlignRight(int r)
{
	RECT rt;
	GetRect(rt);
	int dst = r - rt.right;
	MoveBy(dst, 0);
}


void PanelItem::AlignTop(int t)
{
	RECT r;
	GetRect(r);
	int dst = t - r.top;
	MoveBy(0, dst);
}


void PanelItem::AlignBottom(int b)
{
	RECT r;
	GetRect(r);
	int dst = b - r.bottom;
	MoveBy(0, dst);
}


void PanelItem::HCenter(int hc)
{
	RECT r;
	GetRect(r);
	int dst = hc - (r.left+r.right)/2;
	MoveBy(dst, 0);
}


void PanelItem::VCenter(int vc)
{
	RECT r;
	GetRect(r);
	int dst = vc - (r.top+r.bottom)/2;
	MoveBy(0, dst);
}


int PanelItem::VertSortFunc(const void* pV1, const void* pV2)
{
	const PanelItem** pI1 = (const PanelItem**)(pV1);
	const PanelItem** pI2 = (const PanelItem**)(pV2);
	return (*pI1)->_origin.y - (*pI2)->_origin.y;
}


int PanelItem::HorzSortFunc(const void* pV1, const void* pV2)
{
	const PanelItem** pI1 = (const PanelItem**)(pV1);
	const PanelItem** pI2 = (const PanelItem**)(pV2);
	return (*pI1)->_origin.x - (*pI2)->_origin.x;
}


void PanelItem::Properties(const Window& w)
{
	w.MessageBox(IDS_NOPROPERTIESTOSET);
}


void PanelItem::SetName(const char* bf)
{
	strncpy(_name, bf, PanelParameters::eMaxNameLength);
	_name[PanelParameters::eMaxNameLength] = 0;
	_namelen = strlen(_name);
}


bool PanelItem::_Action(int val, bool final) const
{
	assert(0 != _pDesign);
	static char bf[256];
	sprintf(bf, _action, val);
	return _pDesign->Action(bf, final);
}


void PanelItem::DrawSelected(Canvas& cnv) const
{
	RECT r;
	r.left = _origin.x-PanelParameters::dmSelectionMark;
	r.right = _origin.x+PanelParameters::dmSelectionMark;
	r.top = _origin.y-PanelParameters::dmSelectionMark;
	r.bottom = _origin.y+PanelParameters::dmSelectionMark;
	MoveToEx(cnv, r.left, r.top, 0);
	LineTo(cnv, r.right, r.top);
	LineTo(cnv, r.right, r.bottom);
	LineTo(cnv, r.left, r.bottom);
	LineTo(cnv, r.left, r.top);
}


PanelSlider::PanelSlider(int x, int y, const char* nm) : 
PanelItem(x, y, nm)
{
	_min = 0;
	_max = 100;
	_Defval = 66;
	_val = _Defval;
	_outline = RGB(255, 255, 255);
	_fill = RGB(255, 0, 0);
	_type = PanelParameters::pitSlider;
}


PanelSlider::PanelSlider(ReadTextFile& src, int heightfix) : PanelItem(0, 0, "")
{
	std::string line;

	src.Read(line);
	{
		if (strncmp(line.c_str(), "name: ", 6))
			throw Exception(IDERR_READINGPANELSLIDER, src.Filename(), src.Line());
		SetName(line.c_str()+6);
	}

	src.Read(line);
	{
		int r1, g1, b1, r2, g2, b2;
		int fields = sscanf(line.c_str(), "colors: %d,%d,%d %d,%d,%d", &r1, &g1, &b1, &r2, &g2, &b2);
		if (6 == fields)
		{
			_outline = RGB(r1, g1, b1);
			_fill = RGB(r2, g2, b2);
		}
		else
		{
			fields = sscanf(line.c_str(), "colors: %d %d", &r1, &r2);
			if (2 == fields)
			{
				if (r1<0)
					r1 = 0;
				else if (r1>255)
					r1 = 255;
				if (r2<0)
					r2 = 0;
				else if (r2>255)
					r2 = 255;

				_outline = PanelParameters::DefaultColormap(r1 % 256);
				_fill = PanelParameters::DefaultColormap(r2 % 256);
			}
			else
				throw Exception(IDERR_READINGPANELSLIDER, src.Filename(), src.Line());
		}
	}

	src.Read(line);
	{
		if (2 != sscanf(line.c_str(), "origin: %d %d", &(_origin.x), &(_origin.y)))
			throw Exception(IDERR_READINGPANELSLIDER, src.Filename(), src.Line());
		if (-1 != heightfix)
			_origin.y = heightfix - _origin.y;
	}

	src.Read(line);
	{
		if (2 != sscanf(line.c_str(), "min/max: %d %d", &_min, &_max))
			throw Exception(IDERR_READINGPANELSLIDER, src.Filename(), src.Line());
	}

	src.Read(line);
	{
		if (1 != sscanf(line.c_str(), "value: %d", &_Defval))
			throw Exception(IDERR_READINGPANELSLIDER, src.Filename(), src.Line());
		_val = _Defval;
	}

	src.Read(line);
	{
		if (strncmp(line.c_str(), "message: ", 9))
			throw Exception(IDERR_READINGPANELSLIDER, src.Filename(), src.Line());
		SetAction(line.c_str()+9);
	}
	_type = PanelParameters::pitSlider;
}


PanelSlider::~PanelSlider()
{}

//#define EXPR

void PanelSlider::Draw(Canvas& cnv) const
{
#ifdef EXPR
	{
		ObjectHolder sp(cnv, pens3Dset.Hilight());
		cnv.MoveTo(_origin.x+PanelParameters::dmSliderWidth-1, _origin.y);
		cnv.LineTo(_origin.x, _origin.y);
		cnv.LineTo(_origin.x, _origin.y+PanelParameters::dmSliderHeight);
		cnv.MoveTo(_origin.x+5, _origin.y+PanelParameters::dmSliderHeight-5);
		cnv.LineTo(_origin.x+PanelParameters::dmSliderWidth-5, _origin.y+PanelParameters::dmSliderHeight-5);
		cnv.LineTo(_origin.x+PanelParameters::dmSliderWidth-5, _origin.y+4);
	}
	{
		ObjectHolder sp(cnv, pens3Dset.Light());
		cnv.MoveTo(_origin.x+PanelParameters::dmSliderWidth-2, _origin.y+1);
		cnv.LineTo(_origin.x+1, _origin.y+1);
		cnv.LineTo(_origin.x+1, _origin.y+PanelParameters::dmSliderHeight-1);
		cnv.MoveTo(_origin.x+4, _origin.y+PanelParameters::dmSliderHeight-4);
		cnv.LineTo(_origin.x+PanelParameters::dmSliderWidth-4, _origin.y+PanelParameters::dmSliderHeight-4);
		cnv.LineTo(_origin.x+PanelParameters::dmSliderWidth-4, _origin.y+3);
	}
	{
		ObjectHolder sp(cnv, pens3Dset.Face());
		cnv.MoveTo(_origin.x+PanelParameters::dmSliderWidth-3, _origin.y+2);
		cnv.LineTo(_origin.x+2, _origin.y+2);
		cnv.LineTo(_origin.x+2, _origin.y+PanelParameters::dmSliderHeight-2);
		cnv.MoveTo(_origin.x+3, _origin.y+PanelParameters::dmSliderHeight-3);
		cnv.LineTo(_origin.x+PanelParameters::dmSliderWidth-3, _origin.y+PanelParameters::dmSliderHeight-3);
		cnv.LineTo(_origin.x+PanelParameters::dmSliderWidth-3, _origin.y+2);
	}
	{
		ObjectHolder sp(cnv, pens3Dset.Shadow());
		cnv.MoveTo(_origin.x+PanelParameters::dmSliderWidth-4, _origin.y+3);
		cnv.LineTo(_origin.x+3, _origin.y+3);
		cnv.LineTo(_origin.x+3, _origin.y+PanelParameters::dmSliderHeight-3);
		cnv.MoveTo(_origin.x+2, _origin.y+PanelParameters::dmSliderHeight-2);
		cnv.LineTo(_origin.x+PanelParameters::dmSliderWidth-2, _origin.y+PanelParameters::dmSliderHeight-2);
		cnv.LineTo(_origin.x+PanelParameters::dmSliderWidth-2, _origin.y+1);
	}
	{
		ObjectHolder sp(cnv, pens3Dset.DkShadow());
		cnv.MoveTo(_origin.x+PanelParameters::dmSliderWidth-5, _origin.y+4);
		cnv.LineTo(_origin.x+4, _origin.y+4);
		cnv.LineTo(_origin.x+4, _origin.y+PanelParameters::dmSliderHeight-4);
		cnv.MoveTo(_origin.x+1, _origin.y+PanelParameters::dmSliderHeight-1);
		cnv.LineTo(_origin.x+PanelParameters::dmSliderWidth-1, _origin.y+PanelParameters::dmSliderHeight-1);
		cnv.LineTo(_origin.x+PanelParameters::dmSliderWidth-1, _origin.y);
	}
	{
		RECT r = 
		{ 
			_origin.x+5, 
				_origin.y+5, 
				_origin.x+2*PanelParameters::dmSliderWidth/3, 
				_origin.y+PanelParameters::dmSliderHeight-5
		};
		int l = (PanelParameters::dmSliderWidth-10)*(_val-_min)/(_max-_min);
		r.right = r.left + l;
		cnv.FilledRectangle(r, _fill);
	}
#else
	{
		Pen op(_outline);
		ObjectHolder sop(cnv, op);
		cnv.Rectangle
			(
			_origin.x, 
			_origin.y, 
			_origin.x+PanelParameters::dmSliderWidth-1, 
			_origin.y+PanelParameters::dmSliderHeight-1
			);
	}
	{
		RECT r = 
		{ 
			_origin.x+1, 
				_origin.y+1, 
				_origin.x+2*PanelParameters::dmSliderWidth/3, 
				_origin.y+PanelParameters::dmSliderHeight-1
		};
		int l = (PanelParameters::dmSliderWidth-2)*(_val-_min)/(_max-_min);
		r.right = r.left + l;
		cnv.FilledRectangle(r, _fill);
	}
	{
		cnv.TextAlign(Canvas::taTop, Canvas::taCenter);
		cnv.BkMode(Canvas::bkTransparent);
		cnv.TextColor(_outline);
		cnv.TextOut
			(
			_origin.x + PanelParameters::dmSliderWidth/2, 
			_origin.y + PanelParameters::dmSliderHeight+4, 
			GetName()
			);
		cnv.TextAlign(Canvas::taBaseline, Canvas::taCenter);
		char bf[16];
		sprintf(bf, "%d", _val);
		cnv.TextOut
			(
			_origin.x + PanelParameters::dmSliderWidth/2, 
			_origin.y - 2, 
			bf
			);
	}
#endif
}


void PanelSlider::DrawInGallery(Canvas& cnv) const
{
	{
		COLORREF o = RGB(GetRValue(_outline)/2, GetGValue(_outline)/2, GetBValue(_outline)/2);
		
		Pen op(o);
		ObjectHolder sop(cnv, op);
		cnv.Rectangle
			(_origin.x/GalleryScale, _origin.y/GalleryScale,
			(_origin.x+PanelParameters::dmSliderWidth-1)/GalleryScale,
			(_origin.y+PanelParameters::dmSliderHeight-1)/GalleryScale);
	}
	{
		COLORREF f = RGB(GetRValue(_fill)/2, GetGValue(_fill)/2, GetBValue(_fill)/2);
		RECT r =
		{ 
			_origin.x+1, 
				_origin.y+1, 
				_origin.x+2*PanelParameters::dmSliderWidth/3, 
				_origin.y+PanelParameters::dmSliderHeight-1
		};
		int l = (PanelParameters::dmSliderWidth-2)*(_val-_min)/(_max-_min);
		r.right = r.left + l;
		r.left /= GalleryScale;
		r.right /= GalleryScale;
		r.top /= GalleryScale;
		r.bottom /= GalleryScale;
		cnv.FilledRectangle(r, f);
	}
}

void PanelSlider::Generate(WriteTextFile& trg) const
{
	trg.WriteLn("type: SLIDER");
	trg.PrintF("name: %s\n", GetName());
	
	{
		BYTE r = GetRValue(_outline);
		BYTE g = GetGValue(_outline);
		BYTE b = GetBValue(_outline);

		trg.PrintF("colors: %d,%d,%d ", r, g, b);

		r = GetRValue(_fill);
		g = GetGValue(_fill);
		b = GetBValue(_fill);

		trg.PrintF("%d,%d,%d\n", r, g, b);
	}

	trg.PrintF("origin: %d %d\n", _origin.x, _origin.y);
	trg.PrintF("min/max: %d %d\n", _min, _max);
	trg.PrintF("value: %d\n", _Defval);
	trg.PrintF("message: %s\n", Action());
}


PanelItem* PanelSlider::Clone() const
{
	PanelSlider* pRes = new PanelSlider(_origin.x, _origin.y, GetName());
	pRes->_min = _min;
	pRes->_max = _max;
	pRes->_Defval = _Defval;
	pRes->_val = _val;
	pRes->_outline = _outline;
	pRes->_fill = _fill;
	pRes->SetAction(Action());

	return pRes;
}


bool PanelSlider::Contains(int x, int y) const
{
	if (x<_origin.x)
		return false;
	if (x>_origin.x+PanelParameters::dmSliderWidth-1)
		return false;
	if (y<_origin.y)
		return false;
	if (y>_origin.y+PanelParameters::dmSliderHeight-1)
		return false;
	return true;
}


void PanelSlider::GetRect(RECT& r) const
{
	r.left = _origin.x;
	r.top = _origin.y;
	r.bottom = r.top + PanelParameters::dmSliderHeight;
	r.right = r.left + PanelParameters::dmSliderWidth;
}


bool PanelSlider::ContainedIn(RECT r) const
{
	if (r.left>_origin.x)
		return false;
	if (r.top>_origin.y)
		return false;
	if (r.right<_origin.x+PanelParameters::dmSliderWidth-1)
		return false;
	if (r.bottom<_origin.y+PanelParameters::dmSliderHeight-1)
		return false;
	return true;
}


void PanelSlider::Properties(const Window& w)
{
	PanelSliderPropDlg dlg(this);
	if (IDOK==dlg.DoModal(w))
	{
		SetName(dlg.Name());
		_min = dlg.MinV();
		_max = dlg.MaxV();
		_Defval = dlg.Val();
		_val = dlg.Val();
		_outline = dlg.Outline();
		_fill = dlg.Fill();
		SetAction(dlg.Action());
	}
}


bool PanelSlider::Hit(Canvas& cnv, int x, int, bool final)
{
	const int dx = x - _origin.x - 1;
	const float rval = static_cast<float>(dx)/(PanelParameters::dmSliderWidth-2);
	const int rng = _max-_min;

	int val = _min + static_cast<int>(rval*rng + 0.5f);

	if (val == _val && !final)
		return true;
	else
		_val = val;

	if (_val<_min)
		_val = _min;
	else if (_val > _max)
		_val = _max;

	{
		HBRUSH hBr = (HBRUSH) GetStockObject(BLACK_BRUSH);
		RECT r;
		r.left = _origin.x;
		r.top = _origin.y - PanelParameters::eFontHeight;
		r.right = r.left + PanelParameters::dmSliderWidth;
		r.bottom = r.top + PanelParameters::dmSliderHeight + 4 + PanelParameters::eFontHeight;
		FillRect(cnv, &r, hBr);
	}

	Draw(cnv);
	::GdiFlush();
	return _Action(_val, final);
}


void PanelSlider::DefaultValue(Canvas& cnv)
{
	if (_val != _Defval)
	{
		_val = _Defval;
		{
			HBRUSH hBr = (HBRUSH) GetStockObject(BLACK_BRUSH);
			RECT r;
			r.left = _origin.x;
			r.top = _origin.y - PanelParameters::eFontHeight;
			r.right = r.left + PanelParameters::dmSliderWidth;
			r.bottom = r.top + PanelParameters::dmSliderHeight + 4 + PanelParameters::eFontHeight;
			FillRect(cnv, &r, hBr);
		}

		Draw(cnv);
		::GdiFlush();
		_Action(_val, false);
	}
}


int PanelButton::_counter = 1;

PanelButton::PanelButton(int x, int y, const char* nm) : 
PanelItem(x, y, nm)
{
	_outline = RGB(255, 255, 255);
	_fill = RGB(0, 0, 255);
	_Defstate = PanelParameters::bsOn;
	_state = _Defstate;
	_type = PanelParameters::pitButton;
}


PanelButton::PanelButton(ReadTextFile& src, int heightfix) : PanelItem(0, 0, "")
{
	std::string line;

	src.Read(line);
	{
		if (strncmp(line.c_str(), "name: ", 6))
			throw Exception(IDERR_READINGPANELSLIDER, src.Filename(), src.Line());
		SetName(line.c_str()+6);
	}

	src.Read(line);
	{
		int r1, g1, b1, r2, g2, b2;
		int fields = sscanf(line.c_str(), "colors: %d,%d,%d %d,%d,%d", &r1, &g1, &b1, &r2, &g2, &b2);
		if (6 == fields)
		{
			_outline = RGB(r1, g1, b1);
			_fill = RGB(r2, g2, b2);
		}
		else
		{
			fields = sscanf(line.c_str(), "colors: %d %d", &r1, &r2);
			if (2 == fields)
			{
				if (r1<0)
					r1 = 0;
				else if (r1>255)
					r1 = 255;
				if (r2<0)
					r2 = 0;
				else if (r2>255)
					r2 = 255;
				_outline = PanelParameters::DefaultColormap(r1 % 256);
				_fill = PanelParameters::DefaultColormap(r2 % 256);
			}
			else
				throw Exception(IDERR_READINGPANELBUTTON, src.Filename(), src.Line());
		}
	}

	src.Read(line);
	{
		if (2 != sscanf(line.c_str(), "origin: %d %d", &(_origin.x), &(_origin.y)))
			throw Exception(IDERR_READINGPANELBUTTON, src.Filename(), src.Line());
		if (-1 != heightfix)
			_origin.y = heightfix - _origin.y;
	}

	src.Read(line);
	{
		int val;
		if (1 != sscanf(line.c_str(), "value: %d", &val))
			throw Exception(IDERR_READINGPANELBUTTON, src.Filename(), src.Line());
		switch (val)
		{
		case 0 :
			_Defstate = PanelParameters::bsOff;
			break;
		case 1 :
			_Defstate = PanelParameters::bsOn;
			break;
		case -1 :
			_Defstate = PanelParameters::bsMonostable;
			break;
		default:
			throw Exception(IDERR_READINGPANELBUTTON, src.Filename(), src.Line());
		}
		_state = _Defstate;
	}

	src.Read(line);
	{
		if (strncmp(line.c_str(), "message: ", 9))
			throw Exception(IDERR_READINGPANELSLIDER, src.Filename(), src.Line());
		SetAction(line.c_str()+9);
	}
	_type = PanelParameters::pitButton;
}


const char* PanelButton::GetUniqueName()
{
	static char bf[PanelParameters::eMaxNameLength + 1];
	sprintf(bf, "Button%d", _counter);
	++_counter;
	return bf;
}

PanelButton::~PanelButton()
{}


void PanelButton::Draw(Canvas& cnv) const
{
#ifdef EXPR
	{
		ObjectHolder sp(cnv, pens3Dset.Hilight());
		cnv.MoveTo(_origin.x+PanelParameters::dmButtonWidth-1, _origin.y);
		cnv.LineTo(_origin.x, _origin.y);
		cnv.LineTo(_origin.x, _origin.y+PanelParameters::dmButtonHeight);
	}
	{
		ObjectHolder sp(cnv, pens3Dset.Light());
		cnv.MoveTo(_origin.x+PanelParameters::dmButtonWidth-2, _origin.y+1);
		cnv.LineTo(_origin.x+1, _origin.y+1);
		cnv.LineTo(_origin.x+1, _origin.y+PanelParameters::dmButtonHeight-1);
	}
	{
		cnv.TextAlign(Canvas::taTop, Canvas::taCenter);
		cnv.BkMode(Canvas::bkTransparent);
		cnv.TextColor(_outline);
		cnv.TextOut
			(
			_origin.x + PanelParameters::dmButtonWidth/2, 
			_origin.y + 4, 
			GetName(), NameLength()
			);
	}
#else
	{
		Pen op(_outline);
		ObjectHolder sop(cnv, op);
		cnv.Rectangle
			(
			_origin.x, 
			_origin.y, 
			_origin.x+PanelParameters::dmButtonWidth-1, 
			_origin.y+PanelParameters::dmButtonHeight-1
			);
	}

	if (_state == PanelParameters::bsOn)
	{
		RECT r = 
		{ 
			_origin.x+1, 
				_origin.y+1, 
				_origin.x+PanelParameters::dmButtonWidth-1, 
				_origin.y+PanelParameters::dmButtonHeight-1
		};
		cnv.FilledRectangle(r, _fill);
	}

	{
		cnv.TextAlign(Canvas::taTop, Canvas::taCenter);
		cnv.BkMode(Canvas::bkTransparent);
		cnv.TextColor(_outline);
		cnv.TextOut
			(
			_origin.x + PanelParameters::dmButtonWidth/2, 
			_origin.y + 4, 
			GetName()
			);
	}
#endif
}

void PanelButton::DrawInGallery(Canvas& cnv) const
{
	{
		COLORREF o = RGB(GetRValue(_outline)/2, GetGValue(_outline)/2, GetBValue(_outline)/2);
		Pen op(o);
		ObjectHolder sop(cnv, op);
		cnv.Rectangle
			(
			_origin.x/GalleryScale, 
			_origin.y/GalleryScale, 
			(_origin.x+PanelParameters::dmButtonWidth-1)/GalleryScale, 
			(_origin.y+PanelParameters::dmButtonHeight-1)/GalleryScale
			);
	}
	{
		RECT r = 
		{ 
			(_origin.x+1)/GalleryScale, 
			(_origin.y+1)/GalleryScale, 
			(_origin.x+PanelParameters::dmButtonWidth-1)/GalleryScale, 
			(_origin.y+PanelParameters::dmButtonHeight-1)/GalleryScale
		};
		COLORREF f = RGB(GetRValue(_fill)/2, GetGValue(_fill)/2, GetBValue(_fill)/2);
		cnv.FilledRectangle(r, f);
	}
}

void PanelButton::Generate(WriteTextFile& trg) const
{
	trg.WriteLn("type: BUTTON");
	trg.PrintF("name: %s\n", GetName());
	
	{
		BYTE r = GetRValue(_outline);
		BYTE g = GetGValue(_outline);
		BYTE b = GetBValue(_outline);

		trg.PrintF("colors: %d,%d,%d ", r, g, b);

		r = GetRValue(_fill);
		g = GetGValue(_fill);
		b = GetBValue(_fill);

		trg.PrintF("%d,%d,%d\n", r, g, b);
	}

	trg.PrintF("origin: %d %d\n", _origin.x, _origin.y);

	trg.Write("value: ");
	switch (_Defstate)
	{
	case PanelParameters::bsOn :
		trg.WriteLn("1");
		break;
	case PanelParameters::bsOff :
		trg.WriteLn("0");
		break;
	case PanelParameters::bsMonostable :
		trg.WriteLn("-1");
		break;
	}
	trg.PrintF("message: %s\n", Action());	
}


PanelItem* PanelButton::Clone() const
{
	PanelButton* pRes = new PanelButton(_origin.x, _origin.y, GetName());

	pRes->_outline = _outline;
	pRes->_fill = _fill;
	pRes->_Defstate = _Defstate;
	pRes->_state = _state;
	pRes->SetAction(Action());

	return pRes;
}


bool PanelButton::Contains(int x, int y) const
{
	if (x<_origin.x)
		return false;
	if (x>_origin.x+PanelParameters::dmButtonWidth-1)
		return false;
	if (y<_origin.y)
		return false;
	if (y>_origin.y+PanelParameters::dmButtonHeight-1)
		return false;
	return true;
}


void PanelButton::GetRect(RECT& r) const
{
	r.left = _origin.x;
	r.top = _origin.y;
	r.bottom = r.top + PanelParameters::dmButtonHeight;
	r.right = r.left + PanelParameters::dmButtonWidth;
}



bool PanelButton::ContainedIn(RECT r) const
{
	if (r.left>_origin.x)
		return false;
	if (r.top>_origin.y)
		return false;
	if (r.right<_origin.x+PanelParameters::dmButtonWidth-1)
		return false;
	if (r.bottom<_origin.y+PanelParameters::dmButtonHeight-1)
		return false;
	return true;
}

void PanelButton::Properties(const Window& w)
{
	PanelButtonPropDlg dlg(this);
	if (IDOK==dlg.DoModal(w))
	{
		SetName(dlg.Name());
		_outline = dlg.Outline();
		_fill = dlg.Fill();
		_Defstate = dlg.Value();
		_state = _Defstate;
		SetAction(dlg.Action());
	}
}


bool PanelButton::Hit(Canvas& cnv, int, int, bool final)
{
	switch (_state)
	{
	case PanelParameters::bsOn :
		_TurnOff(cnv);
		break;
	case PanelParameters::bsOff :
		_TurnOn(cnv);
		break;
	case PanelParameters::bsMonostable :
		_Pulse(cnv);
		break;
	}
	::GdiFlush();
	return _Action(_state, final);
}

void PanelButton::DefaultValue(Canvas& cnv)
{
	if (_state != _Defstate)
	{
		switch (_Defstate)
		{
		case PanelParameters::bsOn :
			_TurnOn(cnv);
			break;
		case PanelParameters::bsOff :
			_TurnOff(cnv);
			break;
		}
		::GdiFlush();
		_Action(_state, false);
	}
}

void PanelButton::_Pulse(Canvas& cnv)
{
	RECT r = 
	{ 
		_origin.x, 
			_origin.y, 
			_origin.x + PanelParameters::dmButtonWidth, 
			_origin.y + PanelParameters::dmButtonHeight
	};

	InvertRect(cnv, &r);
	GdiFlush();
	Sleep(100);
	InvertRect(cnv, &r);
}


void PanelButton::_TurnOff(Canvas& cnv)
{
	{
		HBRUSH hBr = (HBRUSH) GetStockObject(BLACK_BRUSH);
		RECT r =
		{
			_origin.x + 1,
				_origin.y + 1,
				_origin.x + PanelParameters::dmButtonWidth - 1,
				_origin.y + PanelParameters::dmButtonHeight - 1
		};

		FillRect(cnv, &r, hBr);
	}
	{
		cnv.TextAlign(Canvas::taTop, Canvas::taCenter);
		cnv.BkMode(Canvas::bkTransparent);
		cnv.TextColor(_outline);
		cnv.TextOut
			(
			_origin.x + PanelParameters::dmButtonWidth/2, 
			_origin.y + 4, 
			GetName()
			);
	}

	_state = PanelParameters::bsOff;
}

void PanelButton::Reset(Canvas& cnv)
{
	if (IsOn())
	{
		_TurnOff(cnv);
		::GdiFlush();
		_Action(_state, false);
	}
}

void PanelButton::_TurnOn(Canvas& cnv)
{
	{
		RECT r =
		{
			_origin.x + 1,
				_origin.y + 1,
				_origin.x + PanelParameters::dmButtonWidth - 1,
				_origin.y + PanelParameters::dmButtonHeight - 1
		};

		cnv.FilledRectangle(r, _fill);
	}
	{
		cnv.TextAlign(Canvas::taTop, Canvas::taCenter);
		cnv.BkMode(Canvas::bkTransparent);
		cnv.TextColor(_outline);
		cnv.TextOut
			(
			_origin.x + PanelParameters::dmButtonWidth/2, 
			_origin.y + 4, 
			GetName()
			);
	}

	_state = PanelParameters::bsOn;
}



PanelLabel::PanelLabel(int x, int y, const char* nm) : 
PanelItem(x, y, nm)
{
	_color = RGB(255, 255, 0);
	_type = PanelParameters::pitLabel;
	MeasureSize();
}

PanelLabel::PanelLabel(ReadTextFile& src, int heightfix) : PanelItem(0, 0, "")
{
	std::string line;

	src.Read(line);
	{
		if (strncmp(line.c_str(), "name: ", 6))
			throw Exception(IDERR_READINGPANELSLIDER, src.Filename(), src.Line());
		SetName(line.c_str()+6);
	}

	src.Read(line);
	{
		int r, g, b;
		int fields = sscanf(line.c_str(), "color: %d,%d,%d", &r, &g, &b);

		if (3 == fields)
		{
			_color = RGB(r, g, b);
		}
		else
		{
			fields = sscanf(line.c_str(), "color: %d", &r);
			if (1 == fields)
			{
				if (r<0)
					r = 0;
				else if (r>255)
					r = 255;
				_color = PanelParameters::DefaultColormap(r % 256);
			}
			else
				throw Exception(IDERR_READINGPANELLABEL, src.Filename(), src.Line());
		}
	}

	src.Read(line);
	{
		if (2 != sscanf(line.c_str(), "origin: %d %d", &(_origin.x), &(_origin.y)))
			throw Exception(IDERR_READINGPANELBUTTON, src.Filename(), src.Line());
		if (-1 != heightfix)
			_origin.y = heightfix - _origin.y;
	}
	_type = PanelParameters::pitLabel;
	MeasureSize();
}


PanelLabel::~PanelLabel()
{}

void PanelLabel::MeasureSize()
{
	DesktopCanvas dc;
	ObjectHolder sf(dc, PanelParameters::PanelFont.HObj());
	dc.MeasureText(GetName(), _size);
}


void PanelLabel::Draw(Canvas& cnv) const
{
	cnv.BkMode(Canvas::bkTransparent);
	cnv.TextColor(_color);
	TextAlignment ta(cnv, TextAlignment::Top, TextAlignment::Left);
	cnv.TextOut(_origin.x, _origin.y, GetName());
}

void PanelLabel::DrawInGallery(Canvas&) const
{}

void PanelLabel::Generate(WriteTextFile& trg) const
{
	trg.WriteLn("type: LABEL");
	trg.PrintF("name: %s\n", GetName());
	
	{
		BYTE r = GetRValue(_color);
		BYTE g = GetGValue(_color);
		BYTE b = GetBValue(_color);

		trg.PrintF("color: %d,%d,%d\n", r, g, b);
	}

	trg.PrintF("origin: %d %d\n", _origin.x, _origin.y);
}



PanelItem* PanelLabel::Clone() const
{
	PanelLabel* pRes = new PanelLabel(_origin.x, _origin.y, GetName());
	pRes->_color = _color;
	pRes->_size  = _size;
	return pRes;
}

void PanelLabel::Properties(const Window& w)
{
	PanelLabelPropDlg dlg(this);
	if (IDOK==dlg.DoModal(w))
	{
		int cx = _origin.x + _Width()/2;
		SetName(dlg.Name());
		MeasureSize();
		int ncx = _origin.x + _Width()/2;
		int mx = ncx-cx;
		if (mx>_origin.x)
			mx = _origin.x;
		MoveBy(-mx, 0);
		_color = dlg.Color();
	}
}

bool PanelLabel::Contains(int x, int y) const
{
	if (x<_origin.x)
		return false;
	if (x>_origin.x+_Width())
		return false;
	if (y<_origin.y)
		return false;
	if (y>_origin.y+_Height())
		return false;
	return true;
}


void PanelLabel::GetRect(RECT& r) const
{
	r.left = _origin.x;
	r.top = _origin.y;
	r.bottom = r.top + _Height();
	r.right = r.left + _Width();
}


bool PanelLabel::ContainedIn(RECT r) const
{
	if (r.left>_origin.x)
		return false;
	if (r.top>_origin.y)
		return false;
	if (r.right<_origin.x+_Width())
		return false;
	if (r.bottom<_origin.y+_Height())
		return false;
	return true;
}


bool PanelLabel::Hit(Canvas&, int, int, bool)
{
	return true;
}




PanelItem* PanelItem::CreateFromFile(const char* bf, ReadTextFile& src, int hf)
{
	char type[64];
	if (1 != sscanf(bf, "type: %63s", type))
		throw Exception(IDERR_UNKNOWNPANELCTRLTYPE, src.Filename(), src.Line());
	if (!(strcmp(type, "SLIDER")))
		return new PanelSlider(src, hf);
	else if (!(strcmp(type, "BUTTON")))
		return new PanelButton(src, hf);
	else if (!(strcmp(type, "LABEL")))
		return new PanelLabel(src, hf);
	else if (!(strcmp(type, "GROUP")))
		return new PanelGroup(src);
	else
		throw Exception(IDERR_UNKNOWNPANELCTRLTYPE, src.Filename(), src.Line());
}



PanelGroup::PanelGroup() : PanelItem(0, 0, "")
{
	_type = PanelParameters::pitGroup;
	_items = 0;
	_color = RGB(192, 192, 192);
}


PanelGroup::PanelGroup(ReadTextFile& src) : PanelItem(0, 0, "")
{
	_type = PanelParameters::pitGroup;
	_items = 0;
	std::string line;
	src.Read(line);
	int r, g, b;
	int fields = sscanf(line.c_str(), "color: %d,%d,%d", &r, &g, &b);
	if (3 == fields)
	{
		_color = RGB(r, g, b);
	}
	else
	{
		fields = sscanf(line.c_str(), "color: %d", &r);
		if (1 == fields)
		{
			if (r<0)
				r = 0;
			else if (r>255)
				r = 255;
			_color = PanelParameters::DefaultColormap(r % 256);
		}
		else
			throw Exception(IDERR_READINGPANELGROUP, src.Filename(), src.Line());
	}

	src.Read(line);
	while (strcmp(line.c_str(), "ENDGROUP"))
	{
		if (CanAddButton())
		{
			strcpy(_buttons[_items], line.c_str());
			_items++;
		}
		else
			throw Exception(IDERR_PANELGROUPTOOMANYBUTTONS, src.Filename(), src.Line());
		src.Read(line);
	}
}


PanelGroup::~PanelGroup()
{}


void PanelGroup::Generate(WriteTextFile& trg) const
{
	trg.WriteLn("type: GROUP");
	{
		BYTE r = GetRValue(_color);
		BYTE g = GetGValue(_color);
		BYTE b = GetBValue(_color);

		trg.PrintF("color: %d,%d,%d\n", r, g, b);
	}
	for (int i=0; i<Items(); i++)
		trg.WriteLn(_buttons[i]);
	
	trg.WriteLn("ENDGROUP");
}

void PanelGroup::GetRect(RECT& r) const
{
	r.left = _rect.left - PanelParameters::dmGroupMargin;
	r.right = _rect.right + PanelParameters::dmGroupMargin;
	r.top = _rect.top - PanelParameters::dmGroupMargin;
	r.bottom = _rect.bottom + PanelParameters::dmGroupMargin;
}

bool PanelGroup::ContainedIn(RECT r) const
{
	if (r.left>_rect.left - PanelParameters::dmGroupMargin)
		return false;
	if (r.right<_rect.right + PanelParameters::dmGroupMargin)
		return false;
	if (r.top>_rect.top - PanelParameters::dmGroupMargin)
		return false;
	if (r.bottom<_rect.bottom + PanelParameters::dmGroupMargin)
		return false;
	return true;
}

bool PanelGroup::Contains(int x, int y) const
{
	POINT pt = {x, y};
	RECT r;
	r.left = _rect.left - PanelParameters::dmGroupMargin;
	r.right = _rect.right + PanelParameters::dmGroupMargin;
	r.top = _rect.top - PanelParameters::dmGroupMargin;
	r.bottom = _rect.bottom + PanelParameters::dmGroupMargin;

	RECT tstr;
	tstr.left = r.left-2;
	tstr.right = r.right+2;
	tstr.top = r.top-2;
	tstr.bottom = r.top+2;
	if (PtInRect(&tstr, pt))
		return true;

	tstr.top = r.bottom-2;
	tstr.bottom = r.bottom+2;
	if (PtInRect(&tstr, pt))
		return true;

	tstr.left = r.left-2;
	tstr.right = r.left+2;
	tstr.top = r.top-2;
	tstr.bottom = r.bottom+2;
	if (PtInRect(&tstr, pt))
		return true;

	tstr.left = r.right-2;
	tstr.right = r.right+2;
	if (PtInRect(&tstr, pt))
		return true;

	return false;
}


PanelItem* PanelGroup::Clone() const
{
	PanelGroup* pGrp = new PanelGroup;
	pGrp->_color = _color;
	pGrp->_rect = _rect;
	pGrp->_origin = _origin;
	for (int i=0; i<Items(); i++)
		strcpy(pGrp->_buttons[i], _buttons[i]);
	pGrp->_items = _items;
	return pGrp;
}


void PanelGroup::Draw(Canvas& cnv) const
{
	assert(_items>0);

	Pen pen(_color);
	ObjectHolder sp(cnv, pen);
	cnv.Rectangle
		(
		_rect.left-PanelParameters::dmGroupMargin,
		_rect.top-PanelParameters::dmGroupMargin,
		_rect.right+PanelParameters::dmGroupMargin,
		_rect.bottom+PanelParameters::dmGroupMargin
		);
}

void PanelGroup::DrawInGallery(Canvas& cnv) const
{
	COLORREF c = RGB(GetRValue(_color)/2, GetGValue(_color)/2, GetBValue(_color)/2);
	Pen pen(c);
	ObjectHolder sp(cnv, pen);
	cnv.Rectangle
		(
		(_rect.left-PanelParameters::dmGroupMargin)/GalleryScale,
		(_rect.top-PanelParameters::dmGroupMargin)/GalleryScale,
		(_rect.right+PanelParameters::dmGroupMargin)/GalleryScale,
		(_rect.bottom+PanelParameters::dmGroupMargin)/GalleryScale
		);
}

bool PanelGroup::Hit(Canvas&, int, int, bool)
{
	return true;
}


void PanelGroup::AddButton(const PanelItem* pIt)
{
	assert(CanAddButton());
	if (0 == _items)
	{
		pIt->GetRect(_rect);
	}
	else
	{
		RECT r;
		pIt->GetRect(r);
		if (r.left<_rect.left)
			_rect.left = r.left;
		if (r.right>_rect.right)
			_rect.right = r.right;
		if (r.top<_rect.top)
			_rect.top = r.top;
		if (r.bottom>_rect.bottom)
			_rect.bottom = r.bottom;
	}
	_origin.x = _rect.left;
	_origin.y = _rect.top;
	strcpy(_buttons[_items], pIt->GetName());
	_items++;
}


bool PanelGroup::UpdateRect(const pPanelItem* arr, int items)
{
	bool usednms[PanelParameters::eMaxButtonsInGroup];
	for (int i=0; i<PanelParameters::eMaxButtonsInGroup; i++)
		usednms[i] = false;
	bool firstfound = false;
	for (int i=0; i<items; i++)
	{
		// if it is a button 
		if (PanelParameters::pitButton == arr[i]->Type())
		{
			int ix = _FindButton(arr[i]->GetName());
			if (-1 != ix)
			{
				RECT r;
				arr[i]->GetRect(r);
				if (!firstfound)
				{
					_rect = r;
					firstfound = true;
				}
				else
				{
					if (r.left<_rect.left)
						_rect.left = r.left;
					if (r.right>_rect.right)
						_rect.right = r.right;
					if (r.top<_rect.top)
						_rect.top = r.top;
					if (r.bottom>_rect.bottom)
						_rect.bottom = r.bottom;
				}
				usednms[ix] = true;
			}
		}
	}

	for (int i=Items()-1; i>=0; i--)
	{
		if (!usednms[i])
			_DeleteName(i);
	}

	_origin.x = _rect.left;
	_origin.y = _rect.top;

	return (_items>0);
}


void PanelGroup::Properties(const Window& w)
{
	PanelGroupPropDlg dlg(this);
	if (IDOK==dlg.DoModal(w))
	{
		_color = dlg.Color();
	}
}


bool PanelGroup::ContainsButton(const char* nm) const
{
	for (int i=0; i<Items(); i++)
	{
		if (!(strcmp(nm, _buttons[i])))
			return true;
	}
	return false;
}

int PanelGroup::_FindButton(const char* nm) const
{
	for (int i=0; i<Items(); i++)
	{
		if (!(strcmp(nm, _buttons[i])))
			return i;
	}
	return -1;
}


void PanelGroup::_DeleteName(int ix)
{
	assert(ix>=0);
	assert(ix<Items());
	for (int i=ix; i<Items()-1; i++)
		strcpy(_buttons[i], _buttons[i+1]);
	_items--;
}

