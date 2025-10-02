#ifndef __STDOUT_H__
#define __STDOUT_H__

/*
class StdOut : public Ctrl
{
public:
	StdOut(HWND, const CREATESTRUCT*);
	~StdOut();
	static void Register(HINSTANCE);
	static HWND Create(HWND, HINSTANCE, int);

	// Message handlers
	bool Size(SizeState, int, int);
	bool Paint();
	bool VScroll(VScrollCode, int, HWND);

	void AddLine(const TCHAR*);
	void Clear();
private:

	HFONT _hFont;
	int _LineHeight;
	int _CharWidth;
	int _NumOfLines;
	int _FirstLine;
	int _FirstCol;
	int _MaxFirstLine;
	int _MaxFirstCol;

	std::string* _aStrings;

	void _VScroll(int);
	enum
	{ 
		MaxChars = 80,
			MaxLines = 128
	};

	static const TCHAR* _ClassName()
	{ return __TEXT("StdOutClass"); }
	DECLARE_COUNTER;
};
*/

#else
	#error File already included
#endif
