/**************************************************************************

  File:		bitmap.h
  Created:	11-Dec-97


  Declaration of class Bitmap


**************************************************************************/


#ifndef __BITMAP_H__
#define __BITMAP_H__


class Bitmap
{
public:
	Bitmap(HBITMAP hBmp = 0)
	{ _hBmp = hBmp; }
	Bitmap(HINSTANCE, const char*);
	Bitmap(const std::string&, HDC, BITMAPINFOHEADER* = 0);
	~Bitmap();
	HBITMAP Handle() const
	{ return _hBmp; }
	void Draw(HDC, int, int);
	void Draw(HDC, int, int, int, int);
	void Load(HINSTANCE, LPCTSTR);
	void GetDim(SIZE& sz) const
	{ GetBitmapDimensionEx(_hBmp, &sz); }
	HBITMAP Release()
	{
		HBITMAP hTmp = _hBmp;
		_hBmp = 0;
		return hTmp;
	}
	void Set(HBITMAP hBmp)
	{
		assert(!IsSet());
		_hBmp = hBmp;
	}
	bool IsSet() const
	{ return (0 != _hBmp); }
private:
	HBITMAP _hBmp;
};


class Icon
{
public:
	Icon(HINSTANCE, LPCTSTR);
	~Icon();
	HICON Handle() const
	{ return _hIcon; }
private:
	HICON _hIcon;
};


class BitmapMaker
{
public:
	BitmapMaker(SIZE sz)
	{
		_bmih.biSize = sizeof(BITMAPINFOHEADER);
		_bmih.biWidth = sz.cx;
		_bmih.biHeight = sz.cy;
		_bmih.biPlanes = 1;
		_bmih.biBitCount = 24;
		_bmih.biCompression = BI_RGB;
		_bmih.biSizeImage = 0;
		_bmih.biXPelsPerMeter = 14400;
		_bmih.biYPelsPerMeter = 14400;
		_bmih.biClrUsed = 0;
		_bmih.biClrImportant = 0;

		_bmi.bmiHeader = _bmih;
	}
	HBITMAP Create(HDC hdc, const void* pData)
	{
		return CreateDIBitmap(hdc, &_bmih, CBM_INIT, pData, &_bmi, 0);
	}
private:
	BITMAPINFOHEADER _bmih;
	BITMAPINFO _bmi;
};


class RawMemory;

class DDB
{
public:
	DDB(HDC hdc, int w, int h) : _w(w), _h(h)
	{ _hBmp = CreateCompatibleBitmap(hdc, _w, _h); }
	~DDB()
	{ DeleteObject(_hBmp); }
	operator HBITMAP() const
	{ return _hBmp; }
	void SaveBmp(const std::string& fnm, HDC hdc) const;
	bool SaveRGB(const std::string& fnm, HDC hdc) const;
private:
	void _AdjustBytes(RawMemory& mem, int w, int h) const;
	int _w, _h;
	HBITMAP _hBmp;
};



#endif
