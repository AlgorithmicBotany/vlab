#include <vector>

#include <fw.h>

#include <image.h>

#include "scalable.h"
#include "rgbfile.h"

#include <shstrng.h>


class ImageRead
{
public:
	ImageRead(const char* fname)
	{
		_pImg = iopen(fname, "rb", 0, 0, 0, 0, 0);
		if (0 == _pImg)
			throw Exception(SharedStr::GetLibString(SharedStr::strErrOpenRGBFile), fname);
	}
	~ImageRead()
	{
		iclose(_pImg);
	}
	IMAGE* operator->() 
	{ return _pImg; }
	void ReadRow(unsigned short* bf, int row, int z)
	{ getrow(_pImg, bf, row, z); }
private:
	IMAGE* _pImg;
};

RgbFile::RgbFile(const char* fname)
{
	_data = 0;
	ImageRead img(fname);
	if (img->zsize != 3)
		throw Exception(SharedStr::GetLibString(SharedStr::strErrOnlyRGBFile), fname);
	_w = img->xsize;
	_h = img->ysize;
	std::vector<unsigned short> row(_w);
	_data = (unsigned char*) malloc(3 * _w * _h);
	if (0 == _data)
		throw Exception(SharedStr::GetLibString(SharedStr::strErrOutOfMemory));
	for (int r=0; r<_h; ++r)
	{
		for (int z=0; z<3; ++z)
		{
			img.ReadRow(&(row[0]), r, z);
			for (int x=0; x<_w; ++x)
				Set(x, r, z, static_cast<char>(row[x]));
		}
	}
}


RgbFile::~RgbFile()
{
	if (0 != _data)
		free(_data);
}


COLORREF RgbFile::GetPixel(int x, int y) const
{
	int r = Get(x, y, 0);
	int g = Get(x, y, 1);
	int b = Get(x, y, 2);
	return RGB(r, g, b);
}


void RgbFile::GetLine(int l, Scalable::ImageLine& line) const
{
	line.Clear();
	for (int i=0; i<Width(); ++i)
		line.AddPoint(GetPixel(i, l));
}

