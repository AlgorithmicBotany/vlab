#ifndef __RGBFILE_H__
#define __RGBFILE_H__


class RgbFile
{
public:
	RgbFile(const char*);
	~RgbFile();
	void Set(int x, int y, int z, unsigned char v)
	{ _data[_Addr(x, y, z)] = v; }
	unsigned char Get(int x, int y, int z) const
	{ return _data[_Addr(x, y, z)]; }
	COLORREF GetPixel(int x, int y) const;
	void GetSize(SIZE& sz)
	{
		sz.cx = _w;
		sz.cy = _h;
	}
	int Width() const
	{ return _w; }
	int Height() const
	{ return _h; }
	int Lines() const
	{ return Height(); }
	void GetLine(int, Scalable::ImageLine&) const;
	const unsigned char* Buffer() const
	{ return _data; }
private:
	int _Addr(int x, int y, int z) const
	{ 
		assert(x>=0);
		assert(x<_w);
		assert(y>=0);
		assert(y<_h);
		assert(z>=0 && z<=3);
		return _w*y*3 + x*3 + (2-z);
	}
	int _w;
	int _h;
	unsigned char* _data;
};


#else
#error File already included
#endif
