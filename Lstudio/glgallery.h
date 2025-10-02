#ifndef __GLGALLERY_H__
#define __GLGALLERY_H__


class GLGallery : public OpenGLWindow, public ObjectGallery
{
public:
	GLGallery(HWND, const CREATESTRUCT*, float, int, bool = false);
	~GLGallery();
	
	bool LButtonDown(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool MouseMove(KeyState, int, int);
	bool CaptureChanged();
	bool HScroll(HScrollCode, int, HWND);
	LRESULT Notify(int, const NMHDR*);

	void ForceRepaint()
	{ 
		CurrentContext cc(this);
		_DoPaint(); 
		cc.SwapBuffers();
	}
	void ColorschemeModified();
	void ScrollToShow();
protected:
	void _DoPaint() const;
	void _DoSize();
	void _HandleWinSize();
	virtual void _UpdateScrollbar();
	virtual const TCHAR* _GetObjectName(int) const;

	void _PasteBefore();
	void _PasteAfter();

	// Clipboard support
	HWND _ClipboardOwner() const
	{ return Hwnd(); }

	void _Delete();
	const float _ObjectDim;
	int _DetermineSelection(int, int) const;
	virtual int _DetermineInsertion(int, int) const;
	virtual void _DrawInsertLine(int) const;
	void _DragTo(int);
	int _LastVisible() const
	{ return Width()/_xItemSize + _firstVisible; }
	int _firstVisible;
	int _xItemSize;

	ToolTip _TT;
	const bool _useTT;

	bool _dragging;
	int _dragtarget;

	Cursor _normal;
	Cursor _drag;
};


#else
	#error File already included
#endif
