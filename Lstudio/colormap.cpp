#include <fw.h>

#include "colormap.h"

#include "resource.h"


Colormap::Colormap()
{
	_arr = new COLORREF[NumOfColors];
	Reset();
}


Colormap::~Colormap()
{
	delete []_arr;
}


void Colormap::Reset()
{
	for (int i=0; i<NumOfColors; i++)
		_arr[i] = _InitMap[i];
}


void Colormap::Smooth(int from, int to)
{
	assert(from>=0);
	assert(from<NumOfColors);
	assert(to>=0);
	assert(to<NumOfColors);
	assert(from<=to);
	if (from == to)
		return;

	const int steps = to - from;
	const float RedStart = GetRValue(Get(from))/static_cast<float>(MaxComponentValue);
	const float GreenStart = GetGValue(Get(from))/static_cast<float>(MaxComponentValue);
	const float BlueStart = GetBValue(Get(from))/static_cast<float>(MaxComponentValue);
	const float RedEnd = GetRValue(Get(to))/static_cast<float>(MaxComponentValue);
	const float GreenEnd = GetGValue(Get(to))/static_cast<float>(MaxComponentValue);
	const float BlueEnd = GetBValue(Get(to))/static_cast<float>(MaxComponentValue);
	const float RedStep = (RedEnd-RedStart)/steps;
	const float GreenStep = (GreenEnd-GreenStart)/steps;
	const float BlueStep = (BlueEnd-BlueStart)/steps;

	for (int i=1; i<steps; i++)
	{
		Set(from + i, 
			RGB(
			static_cast<float>(MaxComponentValue)*(RedStart + i*RedStep), 
			static_cast<float>(MaxComponentValue)*(GreenStart + i*GreenStep), 
			static_cast<float>(MaxComponentValue)*(BlueStart + i*BlueStep)));
	}
}


void Colormap::Inverse(int from, int to)
{
	assert(from>=0);
	assert(from<NumOfColors);
	assert(to>=0);
	assert(to<NumOfColors);
	assert(from<=to);
	for (int i=from; i<=to; i++)
	{
		COLORREF newc = RGB(MaxComponentValue, MaxComponentValue, MaxComponentValue);
		newc -= Get(i);
		Set(i, newc);
	}
}


void Colormap::Reverse(int from, int to)
{
	assert(from>=0);
	assert(from<NumOfColors);
	assert(to>=0);
	assert(to<NumOfColors);
	assert(from<=to);

	const int mid = (from + to) / 2;
	int last = to;
	for (int i=from; i<=mid; i++)
	{
		COLORREF tmp = Get(last);
		Set(last, Get(i));
		Set(i, tmp);
		last--;
	}
}


const COLORREF Colormap::_InitMap[NumOfColors] =
{
	RGB(  0,   0,   0), RGB(  0,   0, 255), RGB(  0, 255,   0), RGB(  0, 255, 255),
	RGB(255,   0,   0), RGB(255,   0, 255), RGB(255, 255,   0), RGB(255, 255, 255),	
	RGB(  0,   0, 128), RGB(  0, 128,   0), RGB(  0, 128, 128), RGB(128,   0,   0),
	RGB(128,   0, 128), RGB(128, 128,   0), RGB(128, 128, 128), RGB(128, 128, 255),

	RGB(128, 255, 128), RGB(128, 255, 255), RGB(255, 128, 128), RGB(255, 128, 255),
	RGB(255, 255, 128), RGB(255, 255, 255),	RGB(  0,   0, 192), RGB(  0, 192,   0),
	RGB(  0, 192, 192), RGB(192,   0,   0), RGB(192,   0, 192), RGB(192, 192,   0),
	RGB(192, 192, 192),	RGB(192, 192, 255), RGB(192, 255, 192), RGB(192, 255, 255),

	RGB(255, 192, 192), RGB(255, 192, 255), RGB(255, 255, 192), RGB(255, 255, 255),	
	RGB( 64,  64, 255), RGB( 64, 255,  64), RGB( 64, 255, 255), RGB(255,  64,  64),
	RGB(255,  64, 255), RGB(255, 255,  64), RGB(255, 255, 255), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),

	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0),
	RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0), RGB(  0,   0,   0)
};
