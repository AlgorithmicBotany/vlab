/**************************************************************************

  File:		bitmap.cpp
  Created:	11-Dec-97


  Implementation of class Bitmap


**************************************************************************/



#include <cassert>
#include <cstdlib>
#include <fstream>

#include <string>

#include <windows.h>
#include <tchar.h>

#include <fstream>

#include "warningset.h"

#include "bitmap.h"
#include "exception.h"
#include "canvas.h"
#include "lstring.h"
#include "file.h"
#include "rawmemory.h"
#include "libstrng.h"
#include "writergb.h"


Bitmap::Bitmap(HINSTANCE hInst, const char* name)
{
	_hBmp = LoadBitmap(hInst, name);
	if (0 == _hBmp)
		throw Exception(0, FWStr::LoadingBitmap);
}


Bitmap::Bitmap(const std::string& fname, HDC hdc, BITMAPINFOHEADER* pBmih)
{
	_hBmp = 0;
	ReadBinFile src(fname);
	BITMAPFILEHEADER bmfh;
	src.Read(&bmfh, sizeof(BITMAPFILEHEADER));

	{
		const char tb[2] = { 'B', 'M' };
		const char* tst = (const char*) &(bmfh.bfType);
		if (tst[0] != tb[0])
			throw Exception(0, FWStr::InvalidBMPFile, src.Filename());
		if (tst[1] != tb[1])
			throw Exception(0, FWStr::InvalidBMPFile, src.Filename());
	}

	int DibSize = bmfh.bfSize - sizeof(BITMAPFILEHEADER);
	RawMemory bf(DibSize);
	bf.Read(src);

	const BITMAPINFOHEADER* pBMIH = reinterpret_cast<const BITMAPINFOHEADER*>(bf.Buffer());

	const BITMAPINFO* pBMI = reinterpret_cast<const BITMAPINFO*>(bf.Buffer());

	int NumOfColors = 0;

	if (pBMIH->biSize >= 36) 
		NumOfColors = pBMIH->biClrUsed;

	if (0==NumOfColors)
	{
		if (pBMIH->biBitCount != 24)
			NumOfColors = 1 << pBMIH->biBitCount;
	}

	int DIBits= pBMIH->biSize + NumOfColors*sizeof(RGBQUAD);

	_hBmp = CreateDIBitmap(hdc, 
		pBMIH,
		CBM_INIT,
		bf.Buffer(DIBits),
		pBMI,
		DIB_RGB_COLORS);


	if (0 == _hBmp)
		throw Exception(0, FWStr::InvalidBMPFile, src.Filename());

	if (0 != pBmih)
		memcpy(pBmih, pBMIH, sizeof(BITMAPINFOHEADER));
}



Bitmap::~Bitmap()
{
	if (IsSet())
		DeleteObject(_hBmp);
}


void Bitmap::Load(HINSTANCE hInst, LPCTSTR name)
{
	assert(!IsSet());
	_hBmp = LoadBitmap(hInst, name);
}

void Bitmap::Draw(HDC hdc, int x, int y)
{
	assert(IsSet());
	MemoryCanvas mc(hdc);
	ObjectHolder sb(mc, _hBmp);

	BITMAP bm;
	GetObject(_hBmp, sizeof(BITMAP), &bm);
	POINT size; size.x = bm.bmWidth; size.y = bm.bmHeight;
	DPtoLP(hdc, &size, 1);

	POINT orig; orig.x = orig.y = 0;
	DPtoLP(mc, &orig, 1);

	BitBlt(hdc, x, y, size.x, size.y, mc, orig.x, orig.y, SRCCOPY);
}

void Bitmap::Draw(HDC hdc, int x, int y, int w, int h)
{
	assert(IsSet());
	MemoryCanvas mc(hdc);
	ObjectHolder sb(mc, _hBmp);

	BITMAP bm;
	GetObject(_hBmp, sizeof(BITMAP), &bm);
	POINT size; size.x = bm.bmWidth; size.y = bm.bmHeight;
	DPtoLP(hdc, &size, 1);

	POINT orig; orig.x = orig.y = 0;
	DPtoLP(mc, &orig, 1);

	StretchBlt(hdc, x, y, w, h, mc, orig.x, orig.y, size.x, size.y, SRCCOPY);
}

Icon::Icon(HINSTANCE hInst, LPCTSTR nm)
{
	_hIcon = (HICON)LoadImage(hInst, nm, IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	if (0 == _hIcon)
		throw Exception(0, FWStr::LoadingIcon);
}

Icon::~Icon()
{
	DestroyIcon(_hIcon);
}

void DDB::_AdjustBytes(RawMemory& mem, int w, int h) const
{
	int pad = w*3 % 4;
	assert(pad != 0);
	pad = 4-pad;
	const int rowsz = w*3+pad;
	const char* bfsrc = mem.Buffer()+rowsz;
	char* bftrg = mem.Buffer()+3*w;
	for (int row = 1; row<h; ++row)
	{
		memmove(bftrg, bfsrc, 3*w);
		bfsrc += rowsz;
		bftrg += 3*w;
	}
}

void DDB::SaveBmp(const std::string& fnm, HDC hdc) const
{
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biWidth = _w;
	bmi.bmiHeader.biHeight = _h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	RawMemory mem(_w*_h*4);
	GetDIBits(hdc, _hBmp, 0, _h, mem.Buffer(), &bmi, DIB_RGB_COLORS);

	std::ofstream trg(fnm, std::ios_base::out | std::ios_base::binary);

	BITMAPFILEHEADER bmfh;
	{
		union
		{
			WORD w;
			char c[2];
		} u; u.c[0] = 'B'; u.c[1] = 'M';
		bmfh.bfType = u.w;

		bmfh.bfSize =
			sizeof(BITMAPFILEHEADER) +
			sizeof(BITMAPINFOHEADER) +
			3*_w*_h;
		bmfh.bfReserved1 = 0;
		bmfh.bfReserved2 = 0;
		bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	}

	trg.write(reinterpret_cast<const char*>(&bmfh), sizeof(BITMAPFILEHEADER));

	trg.write(reinterpret_cast<const char*>(&bmi), sizeof(BITMAPINFOHEADER));

	trg.write(mem.Buffer(), mem.Size());
}

bool DDB::SaveRGB(const std::string& fnm, HDC hdc) const
{
	BITMAPINFO bmi;
	bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
	bmi.bmiHeader.biWidth = _w;
	bmi.bmiHeader.biHeight = -_h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;

	RawMemory mem(_w*_h*4);
	GetDIBits(hdc, _hBmp, 0, _h, mem.Buffer(), &bmi, DIB_RGB_COLORS);
	if (0 != (_w*3)%4)
		_AdjustBytes(mem, _w, _h);
	FILE* fp = fopen(fnm.c_str(), "wb");
	if (0 != fp)
	{
		WriteIRIS(fp, reinterpret_cast<unsigned char*>(mem.Buffer()), 2, _w, _h, 0, 0, 0, 102);
		fclose(fp);
		return true;
	}
	else
		return false;
}
