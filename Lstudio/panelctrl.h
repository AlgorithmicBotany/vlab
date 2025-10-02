#ifndef __PANELCTRL_H__
#define __PANELCTRL_H__


class PrjNotifySink;
class PanelDesign;

// This class represents the teared off panels
class PanelCtrl : public Ctrl
{
public:
	PanelCtrl(HWND, const CREATESTRUCT*);
	~PanelCtrl();
	static HWND Create(HINSTANCE, PanelDesign*);
	static void Register(HINSTANCE);

	bool Paint();
	bool LButtonDown(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool MouseMove(KeyState, int, int);
	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);
	bool KillFocus(HWND);
	bool Timer(UINT);

private:

	static const TCHAR* _ClassName()
	{ return __TEXT("PanelCtrl"); }

	void _AdaptCMenu(HMENU);

	void _Reset();

	HFONT _font;

	FW::Timer _timer;
	enum
	{ 
		eTimerId = 1,
		eTimerTimeout = 250
	};

	PanelDesign* _pDsgn;
	std::string _directory;
	POINT _shift;
	bool _dragging;
	POINT _lp;
};


#else
	#error File already included
#endif
