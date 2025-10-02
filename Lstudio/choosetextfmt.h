#ifndef __ChooseTextFormat_H__
#define __ChooseTextFormat_H__


class CameleonBrush : public Brush
{
public:
	CameleonBrush(COLORREF clr) : Brush(clr) {}
	void Change(COLORREF);
};


class CameleonFont : public Font
{
public:
	CameleonFont(const LogFont& lf) : Font(lf) {}
	void Change(const LogFont&);
	HFONT GetFont() const
	{ return reinterpret_cast<HFONT>(GetObj()); }
};


class ChooseTextFormat : public Dialog
{
public:
	ChooseTextFormat(const LogFont& lf, COLORREF bg, COLORREF fg);


	typedef void (*ApplyCallback)(const LOGFONT&, COLORREF, COLORREF, void*);

	void SetApplyCallback(ApplyCallback pClbck, void* pV)
	{ _pApplyClbck = pClbck, _pClbckParam = pV; }

	bool DoInit();
	const LOGFONT& GetLogFont() const
	{ return _lf; }
	bool HandleMsg(HWND, UINT, WPARAM, LPARAM);
	bool Command(int, Window, UINT);

	HBRUSH CtlColor(HWND, HDC, HWND, int);
	COLORREF GetFgColor() const
	{ return _fg.Color(); }
	COLORREF GetBgColor() const
	{ return _bg.Color(); }
private:
	
	static int CALLBACK _FontEnumProc(const LOGFONT*, const TEXTMETRIC*, DWORD, LPARAM);
	int _DoFontEnum(const LOGFONT*, const TEXTMETRIC*, DWORD);
	static int CALLBACK _FontSizeEnumProc(const LOGFONT*, const TEXTMETRIC*, DWORD, LPARAM);
	int _DoFontSizeEnum(const LOGFONT*, const TEXTMETRIC*, DWORD);

	static void _FgChangedCallback(void* pV, COLORREF clr)
	{
		ChooseTextFormat* pSelf = reinterpret_cast<ChooseTextFormat*>(pV);
		pSelf->_DoFgChanged(clr);
	}
	void _DoFgChanged(COLORREF);
	static void _BgChangedCallback(void* pV, COLORREF clr)
	{
		ChooseTextFormat* pSelf = reinterpret_cast<ChooseTextFormat*>(pV);
		pSelf->_DoBgChanged(clr);
	}
	void _DoBgChanged(COLORREF);

	
	void _FontSelected();
	void _SizeSelected();

	void _UpdatePreview();

	void _DrawItem(HWND, OwnerDraw::Draw);
	void _AddTrueTypeFont(const LOGFONT*);
	LOGFONT _lf;
	OwnerDrawButton::ColorChangedCallback _bgcb;
	OwnerDrawButton _bg;
	OwnerDrawButton::ColorChangedCallback _fgcb;
	OwnerDrawButton _fg;
	CameleonFont    _font;
	CameleonBrush   _bgbrush;

	ComboBox _Font;
	ComboBox _Size;
	Static _Preview;

	HDC _hPreviewDC;
	static const int _truetypeSizes[];
	std::vector<int> _Sizes;

	ApplyCallback _pApplyClbck;
	void* _pClbckParam;
};


#else
	#error File already included
#endif
