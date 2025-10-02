/**************************************************************************

  File:		oglcntxt.h
  Created:	11-Dec-97


  Declaration of class OGLContext


**************************************************************************/


#ifndef __OGLCNTXT_H__
#define __OGLCNTXT_H__


class OGLContext
{
public:
	OGLContext(HDC);
	~OGLContext();
	operator HGLRC() const
	{ return _hRC; }
protected:
	HGLRC _hRC;
};

class PFDescriptor
{
public:
	PFDescriptor();
	void DoubleBuffer(bool);
	void RGBA();
	int ChooseFormat(HDC hdc) const
	{ return ChoosePixelFormat(hdc, &_pfd); }
	bool SetFormat(HDC hdc, int i) const
	{ return TRUE==SetPixelFormat(hdc, i, &_pfd); }
private:
	PIXELFORMATDESCRIPTOR _pfd;
};

#endif
