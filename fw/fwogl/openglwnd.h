/**************************************************************************

  File:		openglwnd.h
  Created:	11-Dec-97


  Declaration of class OpenGLWindow


**************************************************************************/


#ifndef __OPENGLWND_H__
#define __OPENGLWND_H__


class OpenGLWindow : public Ctrl
{
	friend class CurrentContext;
public:
	OpenGLWindow(HWND, const CREATESTRUCT*);
	~OpenGLWindow();

	bool Size(SizeState, int w, int h);
	bool Paint();

	void SaveImage(const TCHAR*);
	void Invalidate()
	{ assert(0); }
	Canvas& Cnv() 
	{ return _cnv; }
protected:

	virtual void _DoSize() = 0;
	virtual void _DoPaint() const = 0;

private:
	UpdateCanvas _cnv;
protected:
	OGLContext _cntxt;
};


class CurrentContext : private TmpCanvas
{
public:
	CurrentContext(OpenGLWindow* pWnd) : TmpCanvas(pWnd->Cnv())
	{
#ifdef _DEBUG
		assert(!_exists);
		_exists = true;
#endif
		wglMakeCurrent(GetDC(), pWnd->_cntxt);
	}
	void SwapBuffers()
	{ ::SwapBuffers(GetDC()); }
	~CurrentContext()
	{
		wglMakeCurrent(0, 0);
#ifdef _DEBUG
		_exists = false;
#endif
	}
#ifdef _DEBUG
private:
	static bool _exists;
#endif
};





#endif
