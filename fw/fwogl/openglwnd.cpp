/**************************************************************************

  File:		openglwnd.cpp
  Created:	11-Dec-97


  Implementation of class OpenGLWindow


**************************************************************************/


#include <vector>

#include <fw.h>
#include <gl\gl.h>

#include "oglcntxt.h"
#include "openglwnd.h"


#ifdef _DEBUG
bool CurrentContext::_exists = false;
#endif


OpenGLWindow::OpenGLWindow(HWND hwnd, const CREATESTRUCT* pCS) : 
Ctrl(hwnd, pCS), 
_cnv(Hwnd()), 
_cntxt(_cnv)
{}


OpenGLWindow::~OpenGLWindow()
{}


bool OpenGLWindow::Size(SizeState, int, int)
{
	CurrentContext cc(this);
	_DoSize();
	return true;
}


bool OpenGLWindow::Paint()
{
	ValidateRect(Hwnd(), 0);
	CurrentContext cc(this);
	_DoPaint();
	cc.SwapBuffers();
	return true;
}

void OpenGLWindow::SaveImage(const TCHAR* fname)
{
	CurrentContext cc(this);
	WriteBinFile trg(fname);

	int rowpad = 4 - ((3*Width()) % 4);
	if (4 == rowpad)
		rowpad = 0;

	BITMAPFILEHEADER bmfh;
	{
		union
		{
			WORD w;
			char c[3];
		} u; u.c[0] = 'B'; u.c[1] = 'M';
		bmfh.bfType = u.w;
		bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (3*Width()+rowpad)*Height();
		bmfh.bfReserved1 = 0;
		bmfh.bfReserved2 = 0;
		bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	}
	trg.Write(&bmfh, sizeof(BITMAPFILEHEADER));

	BITMAPINFOHEADER bmih;
	{
		memset(&bmih, 0, sizeof(BITMAPINFOHEADER));
		bmih.biSize = sizeof(BITMAPINFOHEADER);
		bmih.biWidth = Width();
		bmih.biHeight = Height();
		bmih.biPlanes = 1;
		bmih.biBitCount = 24;
	}

	trg.Write(&bmih, sizeof(BITMAPINFOHEADER));

	glReadBuffer(GL_FRONT);

	std::vector<GLubyte> rrow(Width());
	std::vector<GLubyte> grow(Width());
	std::vector<GLubyte> brow(Width());

	for (int y=0; y<Height(); y++)
	{
		BYTE bgr[3];
		glReadPixels(0, y, Width(), 1, GL_RED, GL_UNSIGNED_BYTE, &rrow[0]);
		glReadPixels(0, y, Width(), 1, GL_GREEN, GL_UNSIGNED_BYTE, &grow[0]);
		glReadPixels(0, y, Width(), 1, GL_BLUE, GL_UNSIGNED_BYTE, &brow[0]);
		for (int x=0; x<Width(); x++)
		{
			bgr[2] = rrow[x];
			bgr[1] = grow[x];
			bgr[0] = brow[x];
			trg.Write(bgr, sizeof(BYTE), 3);
		}
		if (rowpad != 0)
			trg.Write(bgr, sizeof(BYTE), rowpad);
	}
}


