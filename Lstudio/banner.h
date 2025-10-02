#ifndef __BANNER_H__
#define __BANNER_H__


class Banner : public Ctrl
{
public:
	static void Register(HINSTANCE);
	static HWND Create(HINSTANCE, HWND);

	Banner(HWND, const CREATESTRUCT*);
	~Banner();

	bool Timer(UINT);
	bool Paint();
	bool Close()
	{ return CanBeClosed(); }


	bool CanBeClosed();
private:

	static const TCHAR* _ClassName()
	{ return __TEXT("LStudioBanner"); }

	Bitmap _Logo;
	Font _font;

	int _delay;
	bool _canbeclosed;

	enum Params
	{
		tDelay = 3,
		tCloseBanner = 4,
		pWidth = 384,
		pHeight = 384,
		pTimerId = 101,
		pDelay = 500
	};
};



#else
	#error File already included
#endif
