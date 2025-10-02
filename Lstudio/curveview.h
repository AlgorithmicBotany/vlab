#ifndef __CURVEVIEW_H__
#define __CURVEVIEW_H__


#include "curveXYZv.h"


class CurveView : public OpenGLWindow, public ObjectView
{
public:
	static void Register(HINSTANCE);
	
	CurveView(HWND, const CREATESTRUCT*);
	~CurveView();

	bool LButtonDown(KeyState, int, int);
	bool MouseMove(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);

    void ForceRepaint() { updateView(); }

	void updateView()
	{
		CurrentContext cc(this);

		_curveXYZView->defaultView(0,0,0,GL_RENDER);

		_DoPaint();
		cc.SwapBuffers();
	}

	CurveXYZView* getCurveXYZView()
	{
		assert(0 != _curveXYZView);

		return _curveXYZView;
	}

private:
	CurveXYZView* _curveXYZView;

	void _DoInit();
	void _DoSize();
	void _DoPaint() const;

	static const TCHAR* _ClassName()
	{ return __TEXT("CurveView"); }
};


#else
	#error File already included
#endif
