#ifndef __OWNERDRAWBUTTON_H__
#define __OWNERDRAWBUTTON_H__


class OwnerDrawButton
{
public:

	typedef void (*ChangedCallback)(void*, COLORREF);
	struct ColorChangedCallback
	{
		ColorChangedCallback(ChangedCallback pClbck, void* pV) 
			: _pClbck(pClbck), _pV(pV) {}
		ColorChangedCallback(ColorChangedCallback* pCB)
		{
			if (0 != pCB)
			{
				_pClbck = pCB->_pClbck;
				_pV = pCB->_pV;
			}
			else
			{
				_pClbck = 0;
				_pV = 0;
			}
		}
		ChangedCallback _pClbck;
		void* _pV;
	};

	OwnerDrawButton(COLORREF, ColorChangedCallback* pCb);
	virtual void DrawItem(OwnerDraw::Draw&) const;
	void SetColor(COLORREF);
	COLORREF Color() const
	{ return _FillClr; }
	void Check()
	{ _checked = true; }
	void Uncheck()
	{ _checked = false; }
	void SelectColor();
	void Reset(Window w)
	{ _hWnd = w.Hwnd(); }
	void SetHwnd(HWND hwnd)
	{ _hWnd = hwnd; }
	void Invalidate()
	{ ::InvalidateRect(_hWnd, 0, FALSE); }
protected:
	virtual void _DrawFace(OwnerDraw::Draw&) const;
	virtual void _DrawFocus(OwnerDraw::Draw&) const;
	virtual void _DrawSelect(OwnerDraw::Draw&) const;

	static void _ColorChangedClbck(void* pV, COLORREF clr)
	{ 
		OwnerDrawButton* pCtrl = reinterpret_cast<OwnerDrawButton*>(pV);
		pCtrl->_DoColorChanged(clr);
	}
	void _DoColorChanged(COLORREF clr);
	static void _PickClosedClbck(void* pV, const RECT& r)
	{
		OwnerDrawButton* pCtrl = reinterpret_cast<OwnerDrawButton*>(pV);
		pCtrl->_DoPickClosed(r);
	}
	void _DoPickClosed(const RECT&);

private:
	HWND _hWnd;
	HWND _hPickColor;
	bool _checked;
	RECT _r;
	COLORREF _FillClr;

	ColorChangedCallback _cb;
};


#endif
