#ifndef __COLORS_H__
#define __COLORS_H__


/*

  R, G and B are [0, 1]

*/

template<typename T>
class ColorHSV;

template<typename T>
class ColorRGB
{
public:
	ColorRGB(COLORREF clr)
	{
		Set(clr);
	}
	void Set(COLORREF clr)
	{
		_rgb[0] = GetRValue(clr)/static_cast<T>(255.0);
		_rgb[1] = GetGValue(clr)/static_cast<T>(255.0);
		_rgb[2] = GetBValue(clr)/static_cast<T>(255.0);
	}
	void Set(const ColorHSV<T>& hsv)
	{
		if (hsv.S()==static_cast<T>(0.0))
		{
			Set(hsv.V(), hsv.V(), hsv.V());
		}
		else
		{
			T h = hsv.H()/static_cast<T>(60.0);
			int i = static_cast<int>(floor(h));
			T f = h-i;
			T p = hsv.V() * (static_cast<T>(1.0) - hsv.S());
			T q = hsv.V() * (static_cast<T>(1.0) - hsv.S() * f);
			T t = hsv.V() * (static_cast<T>(1.0) - hsv.S() * (static_cast<T>(1.0)-f));
			switch (i)
			{
			case 0 : 
				Set(hsv.V(), t, p); 
				break;
			case 1 :
				Set(q, hsv.V(), p);
				break;
			case 2 :
				Set(p, hsv.V(), t);
				break;
			case 3 :
				Set(p, q, hsv.V());
				break;
			case 4 :
				Set(t, p, hsv.V());
				break;
			case 5 :
				Set(hsv.V(), p, q);
				break;
			}
		}
	}

	void Set(T r, T g, T b)
	{ 
		assert(r>=static_cast<T>(0.0));
		assert(r<=static_cast<T>(1.0));
		assert(g>=static_cast<T>(0.0));
		assert(g<=static_cast<T>(1.0));
		assert(b>=static_cast<T>(0.0));
		assert(b<=static_cast<T>(1.0));
		_rgb[0] = r; 
		_rgb[1] = g;
		_rgb[2] = b;
	}
	T R() const
	{ return _rgb[0]; }
	T G() const
	{ return _rgb[1]; }
	T B() const
	{ return _rgb[2]; }
	void R(T r)
	{
		assert(r>=static_cast<T>(0.0));
		assert(r<=static_cast<T>(1.0));
		_rgb[0] = r;
	}
	void G(T g)
	{
		assert(g>=static_cast<T>(0.0));
		assert(g<=static_cast<T>(1.0));
		_rgb[1] = g;
	}
	void B(T b)
	{
		assert(b>=static_cast<T>(0.0));
		assert(b<=static_cast<T>(1.0));
		_rgb[2] = b;
	}
	COLORREF Get() const
	{ 
		return RGB
			(
			static_cast<int>(static_cast<T>(255.0)*_rgb[0]+static_cast<T>(0.5)),
			static_cast<int>(static_cast<T>(255.0)*_rgb[1]+static_cast<T>(0.5)),
			static_cast<int>(static_cast<T>(255.0)*_rgb[2]+static_cast<T>(0.5))
			);
	}
private:
	T _rgb[3];
};


/*

  Hue is [0, 360)
  S and V are [0, 1]

*/

template<typename T>
class ColorHSV
{
public:
	ColorHSV(COLORREF clr) 
	{
		H(static_cast<T>(0.0));
		ColorRGB<T> rgb(clr);
		Set(rgb);
	}
	void Set(const ColorRGB<T>& rgb)
	{
		T mxv = rgb.R();
		T mnv = rgb.R();
		if (mxv<rgb.G())
			mxv = rgb.G();
		if (mnv>rgb.G())
			mnv = rgb.G();
		if (mxv<rgb.B())
			mxv = rgb.B();
		if (mnv>rgb.B())
			mnv = rgb.B();

		V(mxv);
		if (mxv != static_cast<T>(0.0))
			S((mxv-mnv)/mxv);
		else
			S(static_cast<T>(0.0));

		if (S() != static_cast<T>(0.0))
		{
			const T delta = mxv-mnv;
			T h;
			if (rgb.R()==mxv)
				h = (rgb.G()-rgb.B())/delta;
			else if (rgb.G()==mxv)
				h = static_cast<T>(2.0)+(rgb.B()-rgb.R())/delta;
			else /* rgb.B()==mxv */
				h = static_cast<T>(4.0)+(rgb.R()-rgb.G())/delta;
			h *= static_cast<T>(60.0);
			if (h<static_cast<T>(0.0))
				h += static_cast<T>(360.0);
			H(h);
		}
	}

	void H(T h)
	{ 
		assert(h>=static_cast<T>(0.0));
		assert(h<static_cast<T>(360.0));
		_hsv[0] = h; 
	}
	void S(T s)
	{ 
		assert(s>=static_cast<T>(0.0));
		assert(s<=static_cast<T>(1.0));
		_hsv[1] = s; 
	}
	void V(T v)
	{ 
		assert(v>=static_cast<T>(0.0));
		assert(v<=static_cast<T>(1.0));
		_hsv[2] = v; 
	}
	T H() const
	{ return _hsv[0]; }
	T S() const
	{ return _hsv[1]; }
	T V() const
	{ return _hsv[2]; }
	COLORREF GetHueInRGB() const
	{
		const T h = H()/static_cast<T>(60.0);
		const int i = static_cast<int>(floor(h));
		const T f = h-i;
		const T p = static_cast<T>(0.0);
		const T q = static_cast<T>(1.0)-f;
		const T t = f;
		switch (i)
		{
		case 0 : 
			return RGB(255, static_cast<int>(t*255), static_cast<int>(p*255)); 
			break;
		case 1 :
			return RGB(static_cast<int>(q*255), 255, static_cast<int>(p*255));
			break;
		case 2 :
			return RGB(static_cast<int>(p*255), 255, static_cast<int>(t*255));
			break;
		case 3 :
			return RGB(static_cast<int>(p*255), static_cast<int>(q*255), 255);
			break;
		case 4 :
			return RGB(static_cast<int>(t*255), static_cast<int>(p*255), 255);
			break;
		case 5 :
			return RGB(255, static_cast<int>(p*255), static_cast<int>(q*255));
			break;
		}
		assert(0);
		return 0;
	}
private:
	T _hsv[3];
};


typedef ColorRGB<float> ColorRGBf;
typedef ColorHSV<float> ColorHSVf;

#endif
