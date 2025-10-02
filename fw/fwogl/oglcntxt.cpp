/**************************************************************************

  File:		oglcntxt.cpp
  Created:	11-Dec-97


  Declaration of class OGLContext


**************************************************************************/


#include <fw.h>
#include <gl\gl.h>

#include "oglcntxt.h"

#include "liboglstrng.h"

PFDescriptor::PFDescriptor()
{
    _pfd.nSize              = sizeof(PIXELFORMATDESCRIPTOR);
    _pfd.nVersion           = 1;
    _pfd.dwFlags            = PFD_DRAW_TO_WINDOW | 
        PFD_SUPPORT_OPENGL | PFD_GENERIC_FORMAT;
    _pfd.iPixelType         = PFD_TYPE_RGBA;
    _pfd.cColorBits         = 24;
    _pfd.cRedBits           = 0;
    _pfd.cRedShift          = 0;
    _pfd.cGreenBits         = 0;
    _pfd.cGreenShift        = 0;
    _pfd.cBlueBits          = 0;
    _pfd.cBlueShift         = 0;
    _pfd.cAlphaBits         = 32;
    _pfd.cAlphaShift        = 0;
    _pfd.cAccumBits         = 0;
    _pfd.cAccumRedBits      = 0;
    _pfd.cAccumGreenBits    = 0;
    _pfd.cAccumBlueBits     = 0;
    _pfd.cAccumAlphaBits    = 0;
    _pfd.cDepthBits         = 24;
    _pfd.cStencilBits       = 0;
    _pfd.cAuxBuffers        = 0;
    _pfd.iLayerType         = PFD_MAIN_PLANE;
    _pfd.bReserved          = 0;
    _pfd.dwLayerMask        = 0;
    _pfd.dwVisibleMask      = 0;
    _pfd.dwDamageMask       = 0;
}

void PFDescriptor::DoubleBuffer(bool on)
{
	if (on)
		_pfd.dwFlags |= PFD_DOUBLEBUFFER;
	else
		_pfd.dwFlags &= ~PFD_DOUBLEBUFFER;
}


OGLContext::OGLContext(HDC hdc)
{
	PFDescriptor pfd;
	pfd.DoubleBuffer(true);

	int i = pfd.ChooseFormat(hdc);
	if (0==i)
		throw Exception(GetLibOglString(FWOGLStr::InitOpenGL));

	if (!pfd.SetFormat(hdc, i))
		throw Exception(GetLibOglString(FWOGLStr::InitOpenGL));
	
	_hRC = wglCreateContext(hdc);

	if (0 == _hRC)
		throw Exception(GetLibOglString(FWOGLStr::InitOpenGL));
}

OGLContext::~OGLContext()
{
	wglDeleteContext(_hRC);
}



