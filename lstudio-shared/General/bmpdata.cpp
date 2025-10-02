#include <fw.h>

#include "bmpdata.h"
#include "scale.h"

/*

	This code is adapted from xvsmooth.c

*/
void BitmapData::Scale(const unsigned char* src, int w, int h)
{
	if (0 != _a)
		free(_a);
	_a = scale(src, w, h, _w, _h);
}
#ifdef gfejowpg
	if (w==_w && h==_h)
	{
		memcpy(_a, src, 3*_w*_h);
		return;
	}

	void* pNew = realloc(_a, 3*w*h);
	if (0==pNew)
		throw Exception("Out of memory when scaling bitmap");

	if (_w<w && _h<h)
		SmoothScaleXY(src, w, h);
	else if (_w<w && _h>=h)
		SmoothScaleX(src, w, h);
	else if (_w>=w && _h<h)
		SmoothScaleY(src, w, h);
	else
		SmoothScale(src, w, h);
}

void BitmapData::SmoothScale(const byte* src, int w, int h)
{
	std::vector<int> cxtab(_w);
	std::vector<int> pxtab(_w);

	const int Precision = 100;
	const Bpp = 3; // Bytes per pixel

	byte* pp = _a;

	for (int ex=0; ex<_w; ++ex)
	{
		cxtab[ex] = (ex*w)/_w;
		pxtab[ex] = (((ex*w)*Precision)/_w)-(cxtab[ex]*Precision)-Precision/2;
	}

	for (int ey=0; ey<_h; ++ey)
	{
		int cy = (ey*h)/_h;
		int py = (((ey*h)*Precision)/_h)-(cy*Precision) - Precision/2;
		int y1 = 0;
		if (py<0)
		{
			y1 = cy-1;
			if (y1<0)
				y1 = 0;
		}
		else
		{
			y1 = cy+1;
			if (y1>h-1)
				y1 = h-1;
		}
		int cyOff = cy*w*Bpp;
		int y1Off = y1*w*Bpp;
		for (int ex=0; ex<_w; ++ex)
		{
			int cx = cxtab[ex];
			int px = pxtab[ex];
			int x1=0;

			if (px<0)
			{
				x1 = cx-1;
				if (x1<0)
					x1=0;
			}
			else
			{
				x1 = cx+1;
				if (x1>w-1)
					x1=w-1;
			}
			const byte* pptr = src + y1Off + x1*Bpp;
			byte A[3];
			A[0] = pptr[0]; A[1] = pptr[1]; A[2] = pptr[2];

			pptr = src + y1Off + cx*Bpp;
			byte B[3];
			B[0] = pptr[0]; B[1] = pptr[1]; B[2] = pptr[2];

			pptr = src + cyOff + x1*Bpp;
			byte C[3];
			C[0] = pptr[0]; C[1] = pptr[1]; C[2] = pptr[2];

			pptr = src + cyOff + cx*Bpp;
			byte D[3];
			D[0] = pptr[0]; D[1] = pptr[1]; D[2] = pptr[2];

			int apx = abs(px);
			int apy = abs(py);
			int p[4];
			p[0] = (apx*apy)/Precision;
			p[1] = (apy*(Precision-apx))/Precision;
			p[2] = (apx*(Precision-apy))/Precision;
			p[3] = Precision - (p[0]+p[1]+p[2]);

			*pp++ = static_cast<byte>
				((p[0]*A[0])/Precision
				+ (p[1]*B[0])/Precision
				+ (p[2]*C[0])/Precision
				+ (p[3]*D[0])/Precision);

			*pp++ = static_cast<byte>
				((p[0]*A[1])/Precision
				+ (p[1]*B[1])/Precision
				+ (p[2]*C[1])/Precision
				+ (p[3]*D[1])/Precision);

			*pp++ = static_cast<byte>
				((p[0]*A[2])/Precision
				+ (p[1]*B[2])/Precision
				+ (p[2]*C[2])/Precision
				+ (p[3]*D[2])/Precision);
		}
	}
}

void BitmapData::SmoothScaleX(const byte* src, int w, int h)
{
	std::vector<int> lbufR(w);
	std::vector<int> lbufG(w);
	std::vector<int> lbufB(w);
	std::vector<int> pixarr(w+1);
	const int Bpp = 3; // Bytes per pixel

	for (int j=0; j<=w; ++j)
		pixarr[j] = (j*_w + (15*w)/16) / w;
	//const byte* cptr = src;
	//const byte* cptr1 = cptr + _w*Bpp;

	for (int i=0; i<_h; ++i)
	{
		int ypcnt = (((i*h)*64) / _h) - 32;
		if (ypcnt<0)
			ypcnt = 0;
		int pcnt1 = ypcnt & 0x3f;
		int pcnt0 = 64 - pcnt1;

		int thisline = ypcnt>>6;

		const byte* cptr = src + thisline * w * Bpp;
		const byte* cptr1;
		if (thisline+1 < h)
			cptr1 = cptr + w*Bpp;
		else
			cptr1 = cptr;

		for (int j=0; j<w; ++j)
		{
			lbufR[j] = (*cptr * pcnt0 + *cptr1 * pcnt1) >> 6;
			++cptr; ++cptr1;
			lbufR[j] = (*cptr * pcnt0 + *cptr1 * pcnt1) >> 6;
			++cptr; ++cptr1;
			lbufR[j] = (*cptr * pcnt0 + *cptr1 * pcnt1) >> 6;
			++cptr; ++cptr1;
		}
		int pixR = 0;
		int pixG = 0;
		int pixB = 0;
		int pixcnt = 0;
		int lastpix = 0;

		for (int j=0; 
	}
}
#endif
