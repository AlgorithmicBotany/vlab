#ifndef __COLORMAP_H__
#define __COLORMAP_H__


class Colormap
{
public:
	Colormap();
	~Colormap();
	COLORREF Get(int i) const
	{
		assert(i>=0);
		assert(i<NumOfColors);
		return _arr[i];
	}
	void Set(int i, COLORREF clr)
	{
		assert(i>=0);
		assert(i<NumOfColors);
		_arr[i] = clr;
	}
	void Smooth(int, int);
	void Inverse(int, int);
	void Reverse(int, int);

	void SetR(int i, int red)
	{
		assert(i>=0);
		assert(i<NumOfColors);
		assert(red>=0);
		assert(red<=MaxComponentValue);
		COLORREF clrnew = RGB(red, GetGValue(_arr[i]), GetBValue(_arr[i]));
		Set(i, clrnew);
	}

	void SetG(int i, int green)
	{
		assert(i>=0);
		assert(i<NumOfColors);
		assert(green>=0);
		assert(green<=MaxComponentValue);
		COLORREF clrnew = RGB(GetRValue(_arr[i]), green, GetBValue(_arr[i]));
		Set(i, clrnew);
	}
	void SetB(int i, int blue)
	{
		assert(i>=0);
		assert(i<NumOfColors);
		assert(blue>=0);
		assert(blue<=MaxComponentValue);
		COLORREF clrnew = RGB(GetRValue(_arr[i]), GetGValue(_arr[i]), blue);
		Set(i, clrnew);
	}

	void Reset();

	enum 
	{ 
		NumOfColors = 256,
		MaxComponentValue = 255
	};
private:

	COLORREF* _arr;
	static const COLORREF _InitMap[];
};

#else
	#error File already included
#endif
