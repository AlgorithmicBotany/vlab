#ifndef __SPLITTER_H__
#define __SPLITTER_H__


class Splitter : public Ctrl
{
public:
	Splitter(HWND, const CREATESTRUCT*);
	~Splitter();

	static void Register(HINSTANCE);
	static HWND Create(HWND, HINSTANCE, int);
	static const char* _ClassName()
	{ return "FWSplitter"; }

	// Message handlers
	bool Paint();
	bool LButtonDown(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool MouseMove(KeyState, int, int);
	bool CaptureChanged();
private:
	bool _dragging;
	HWND _hParent;
	int _dragStart;
	int _dragY;
};


#endif
