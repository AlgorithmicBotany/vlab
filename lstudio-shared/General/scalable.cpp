#include <vector>

#include <fw.h>

#include "scalable.h"

Scalable::TmpImageLine::TmpImageLine(int i) : _width(i), _arr(3*_width, 0.0f)
{
	_count = 0;
}

void Scalable::TmpImageLine::Add(const Scalable::ImageLine& line)
{
	assert(Width() == line.Width());
	int ix = 0;
	for (int i=0; i<Width(); ++i)
	{
		COLORREF rgb = line.GetPoint(i);
		_arr[ix] += GetRValue(rgb);
		++ix;
		_arr[ix] += GetGValue(rgb);
		++ix;
		_arr[ix] += GetBValue(rgb);
		++ix;
	}
	++_count;
}

void Scalable::TmpImageLine::Reset()
{
	_count = 0;
	for (int i=0; i<Width()*3; ++i)
		_arr[i] = 0;
}


COLORREF Scalable::TmpImageLine::GetAvgRGB(int i) const
{
	assert(i>=0);
	assert(i<Width());
	assert(_count>0);
	float r = _arr[3*i];
	float g = _arr[3*i+1];
	float b = _arr[3*i+2];
	return RGB
		(
		static_cast<int>(r/_count),
		static_cast<int>(g/_count),
		static_cast<int>(b/_count)
		);
}

Scalable::ImageLine::ImageLine(int width, int px, int qx) : 
_width(width), _px(px), _qx(qx), 
_arr(_width)
{
	Clear();
}


void Scalable::ImageLine::AddPoint(COLORREF clr)
{
	assert(_current<_width);
	const float mlt = 1.0f/_qx;
	for (int i=0; i<_px; ++i)
	{
		_cml[0] += GetRValue(clr);
		_cml[1] += GetGValue(clr);
		_cml[2] += GetBValue(clr);
		++_rpt;
		if (_qx==_rpt)
		{
			_arr[_current] = RGB
				(
				static_cast<int>(mlt*_cml[0]),
				static_cast<int>(mlt*_cml[1]),
				static_cast<int>(mlt*_cml[2])
				);
			_rpt = 0;
			_cml[0] = _cml[1] = _cml[2] = 0.0f;
			++_current;
		}
	}
}


void Scalable::ImageLine::Clear()
{
	_current = 0;
	_rpt = 0;
	_cml[0] = _cml[1] = _cml[2] = 0.0f;
	for (int i=0; i<_width; ++i)
		_arr[i] = RGB(0,0,0);
}
