#ifndef __SCALABLE_H__
#define __SCALABLE_H__


namespace Scalable
{


class ImageLine
{
public:
	ImageLine(int width, int px, int qx);
	int Width() const
	{ return _width; }
	COLORREF GetPoint(int i) const
	{
		assert(i>=0);
		assert(i<_width);
		return _arr[i];
	}
	void Clear();
	void AddPoint(COLORREF);
private:
	const int _width;
	const int _px;
	const int _qx;
	std::vector<COLORREF> _arr;
	int _current;
	int _rpt;
	float _cml[3];
};



class TmpImageLine
{
public:
	TmpImageLine(int);
	void Add(const ImageLine&);
	void Reset();
	COLORREF GetAvgRGB(int) const;
	int Width() const
	{ return _width; }
private:
	int _count;
	int _width;
	std::vector<float> _arr;
};


}


#else
#error File already included
#endif
