#ifndef __BMPDATA_H__
#define __BMPDATA_H__

class BitmapData
{
public:
	BitmapData(int w, int h) : _w(w), _h(h)
	{
		assert(0 == _w % 4);
		_a = reinterpret_cast<unsigned char*>(malloc(3*_w*_h));
	}
	~BitmapData()
	{ 
		free(_a); 
	}
	void SetCell(int col, int row, unsigned char r, unsigned char g, unsigned char b)
	{
		assert(col>=0);
		assert(col<_w);
		assert(row>=0);
		assert(row<_h);
		int ix = 3*row*_w + 3*col;
		assert(ix>=0);
		assert(ix<3*_w*_h-2);
		_a[ix++] = b;
		_a[ix++] = g;
		_a[ix] = r;
	}
	void SetCell(int col, int row, COLORREF clr)
	{
		assert(col>=0);
		assert(col<_w);
		assert(row>=0);
		assert(row<_h);
		int ix = 3*row*_w + 3*col;
		assert(ix>=0);
		assert(ix<3*_w*_h-2);
		_a[ix++] = GetBValue(clr);
		_a[ix++] = GetGValue(clr);
		_a[ix] = GetRValue(clr);
	}
	const unsigned char* Buf() const
	{ return _a; }
	int Lines() const
	{ return _h; }
	int Width() const
	{ return _w; }
	template<class T>
		void Scale(const T& src)
	{
		Scale(src.Buffer(), src.Width(), src.Height());
	}
private:
	void Scale(const unsigned char*, int, int);
	typedef unsigned char byte;
	void SmoothScaleXY(const byte*, int, int);
	void SmoothScaleX(const byte*, int, int);
	void SmoothScaleY(const byte*, int, int);
	void SmoothScale(const byte*, int, int);
	int _w, _h;
	unsigned char* _a;
};




#else
#error File already included
#endif
