#ifndef __OWNERDRAW_H__
#define __OWNERDRAW_H__

namespace OwnerDraw
{

class Draw
{
public:
	explicit Draw(const DRAWITEMSTRUCT* pDIS) : _pDIS(pDIS)
	{}
	UINT CtrlId() const
	{ return _pDIS->CtlID; }
	UINT ItemId() const
	{ return _pDIS->itemID; }

	bool IsButton() const
	{ return ODT_BUTTON == _pDIS->CtlType; }

	bool IsSelected() const
	{ return 0 != (_pDIS->itemState & ODS_SELECTED); }

	bool DrawEntire() const
	{ return 0 != (_pDIS->itemAction & ODA_DRAWENTIRE); }
	bool DrawSelect() const
	{ return 0 != (_pDIS->itemAction & ODA_SELECT); }
	bool DrawFocus() const
	{ return 0 != (_pDIS->itemAction & ODA_FOCUS); }
	const RECT& ItemRect() const
	{ return _pDIS->rcItem; }
	HDC DC() const
	{ return _pDIS->hDC; }
private:
	const DRAWITEMSTRUCT* _pDIS;
};

class Measure
{
public:
	explicit Measure(MEASUREITEMSTRUCT* pMIS) : _pMIS(pMIS)
	{}

	bool IsMenu() const
	{ return ODT_MENU == _pMIS->CtlType; }

	void SetWidth(int w)
	{ _pMIS->itemWidth = w; }
	void SetHeight(int h)
	{ _pMIS->itemHeight = h; }
private:
	MEASUREITEMSTRUCT* _pMIS;
};

}


#endif
