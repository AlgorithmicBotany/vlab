#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

/*

  Usage: bmpin scriptfile bmpout

  script file contains commands:

  command1
  command2
  ...

  command is:
  Font name
  flags (bold italic none)
  Font size
  Color
  Text
  Location (x y)
  Alignment

  for example:
  Times New Roman
  Bold
  12
  255 255 255
  Bum tarara
  100 60
  center

  If x==-1 it means that the text should be centered

*/

class Bitmap
{
public:
	Bitmap(const char* bmpfile)
	{
		_hBmp = (HBITMAP) LoadImage
			(
			NULL,
			bmpfile,
			IMAGE_BITMAP,
			0, 0,
			LR_LOADFROMFILE
			);
		DWORD err = GetLastError();
		if (NULL==_hBmp)
			throw "Cannot open image file";
	}
	~Bitmap() { DeleteObject(_hBmp); }
	operator HBITMAP() { return _hBmp; }
	SIZE Dim() const
	{
		SIZE sz;
		BITMAP bm;
		int res = GetObject(_hBmp, 0, NULL);
		res = GetObject(_hBmp, res, &bm);
		DWORD err = GetLastError();
		if (0 == res)
			throw "Cannot determine bitmap size";
		sz.cx = bm.bmWidth;
		sz.cy = bm.bmHeight;
		return sz;
	}
private:
	HBITMAP _hBmp;
};


class CompatibleBmp
{
public:
	CompatibleBmp(HDC hdc, int w, int h)
	{ 
		_hBmp = CreateCompatibleBitmap(hdc, w, h);
		if (NULL == _hBmp)
			throw "Cannot create temporary bitmap";
	}
	~CompatibleBmp()
	{ DeleteObject(_hBmp); }
	operator HBITMAP()
	{ return _hBmp; }
private:
	HBITMAP _hBmp;
};


class MemoryDC
{
public:
	MemoryDC(HBITMAP hbmp)
	{
		_hDC = CreateCompatibleDC(NULL);
		if (NULL == _hDC)
			throw "Cannot create CD";
		if (NULL == SelectObject(_hDC, hbmp))
			throw "Cannot select BMP into DC";
	}
	~MemoryDC() { DeleteDC(_hDC); }
	operator HDC() { return _hDC; }
private:
	HDC _hDC;
};


class SrcFile
{
public:
	SrcFile(const char* fname)
	{
		_fp = fopen(fname, "rt");
		if (NULL == _fp)
			throw "Cannot open text file";
	}
	~SrcFile() { fclose(_fp); }
	operator FILE*() { return _fp; }
private:
	FILE* _fp;
};


class TrgFile
{
public:
	TrgFile(const char* fname)
	{
		_fp = fopen(fname, "wb");
		if (NULL == _fp)
			throw "Cannot create binary file";
	}
	~TrgFile() { fclose(_fp); }
	operator FILE*() { return _fp; }
private:
	FILE* _fp;
};


class Font
{
public:
	Font(const LOGFONT* pLF)
	{
		_hFont = CreateFontIndirect(pLF);
		if (NULL == _hFont)
			throw "Cannot create font";
	}
	~Font() { DeleteObject(_hFont); }
	operator HFONT() { return _hFont; }
private:
	HFONT _hFont;
};


class SelectGDIObj
{
public:
	SelectGDIObj(HDC hdc, HGDIOBJ hObj) : _hdc(hdc)
	{ _hPrevObj = SelectObject(_hdc, hObj); }
	~SelectGDIObj() { SelectObject(_hdc, _hPrevObj); }
private:
	HDC _hdc;
	HGDIOBJ _hPrevObj;
};


class CharArr
{
public:
	CharArr(int sz)
	{ _arr = new char[sz]; }
	~CharArr()
	{ delete []_arr; }
	operator char*()
	{ return _arr; }
private:
	char* _arr;
};


class BitmapInfo
{
public:
	BitmapInfo(BITMAPINFO& src, int palette = 0)
	{ 
		_arr = new char[sizeof(BITMAPINFOHEADER)+palette*sizeof(RGBQUAD)];
		memcpy(_arr, &src, sizeof(BITMAPINFO));
		if (NULL == _arr)
			throw "Out of memory";
	}
	~BitmapInfo()
	{ delete []_arr; }
	operator BITMAPINFO*()
	{ return reinterpret_cast<BITMAPINFO*>(_arr); }
private:
	char* _arr;
};

void UnEscapeString(char* line)
{
  /* Only one type of escaped stuff:
   * \`command to be executed`
   * Two \\s => one.
   */

  //  printf("Turned line %s",line);

  static char newline[1024],cmdline[1024],filename[1024];
  char* nlp = newline, *lp = line;
  char* bkspPos = strchr(line,'\\');
  while(bkspPos != NULL)
  {
    *bkspPos = 0;
    strcpy(nlp,lp);
    nlp += strlen(nlp);
    if(bkspPos[1] == '\\')
    {
      *(nlp++) = '\\';
      lp = bkspPos+2;
    }
    else if(bkspPos[1] == '`')
    {
      char* endCmd = strchr(bkspPos+2,'`');
      *endCmd = 0;
      strcpy(cmdline,bkspPos+2);
      strcpy(cmdline+strlen(cmdline)," > %temp%\\splash.tmp\0");
      system(cmdline);
      strcpy(filename,getenv("temp"));
      strcpy(filename+strlen(filename),"\\splash.tmp\0");
      FILE* inFile = fopen(filename,"r");
      fgets(cmdline,1024,inFile);
      fclose(inFile);
      strcpy(nlp,cmdline);
      nlp += strlen(nlp);
      while(nlp[-1] == '\n' || nlp[-1] == '\r')
	nlp--;
      lp = endCmd + 1;
    }
    else // not a \\ or \` => ignore the backslash
      lp = bkspPos + 1;

    bkspPos = strchr(lp,'\\');
  }
  strcpy(nlp,lp);
  strcpy(line,newline);
  //  printf(" to %s\n",line);
  return;
}

void ExecuteCommand(FILE* src, HDC hdc, SIZE sz)
{
	LOGFONT lf;
	lf.lfHeight = 0;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = FW_DONTCARE;
	lf.lfItalic = FALSE;
	lf.lfUnderline = FALSE;
	lf.lfStrikeOut = FALSE;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH;

	static char line[128];
	fgets(line, 128, src);
	strcpy(lf.lfFaceName, line);

	fgets(line, 128, src);
	if (strstr(line, "bold"))
		lf.lfWeight = FW_BOLD;
	if (strstr(line, "italic"))
		lf.lfItalic = TRUE;

	{
		int h;
		fscanf(src, "%d", &h);
		lf.lfHeight = h;
	}

	{
		int r, g, b;
		fscanf(src, "%d %d %d\n", &r, &g, &b);
		SetTextColor(hdc, RGB(r, g, b));
	}

	static char longline[1024];
	fgets(longline, 1024, src);
	longline[strlen(longline)-1] = 0;
	UnEscapeString(longline);

	int x, y;
	fscanf(src, "%d %d\n", &x, &y);
	if (-1 == x)
		x = sz.cx/2;

	fgets(line, 128, src);
	if (strstr(line, "center"))
		SetTextAlign(hdc, TA_CENTER);
	else if (strstr(line, "left"))
		SetTextAlign(hdc, TA_LEFT);
	else if (strstr(line, "right"))
		SetTextAlign(hdc, TA_RIGHT);

	Font f(&lf);
	SelectGDIObj sf(hdc, f);
	SetBkMode(hdc, TRANSPARENT);
	TextOut(hdc, x, y, longline, strlen(longline));
}

void Execute(FILE* src, HDC hdc, SIZE sz)
{
	while (!feof(src))
		ExecuteCommand(src, hdc, sz);
}

void SaveOutput(const char* trgfn, SIZE sz, HBITMAP hBmp, HDC hdc)
{
	TrgFile trg(trgfn);
	CompatibleBmp cbmp(hdc, sz.cx, sz.cy);
	SelectGDIObj sb(hdc, cbmp);

	BITMAPINFO bmi;
	memset(&(bmi.bmiHeader), 0, sizeof(BITMAPINFOHEADER));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	int res = GetDIBits(hdc, hBmp, 0, 0, NULL, &bmi, DIB_RGB_COLORS);
	DWORD err = GetLastError();
	if (0 == res)
		throw "Cannot manipulate target bitmap";
	CharArr arr(sz.cx*sz.cy);
	SetLastError(0);
	bmi.bmiHeader.biCompression = 0;
	bmi.bmiHeader.biBitCount = 8;
	BitmapInfo bi(bmi, 256);
	BITMAPINFO* pBI = bi;
	res = GetDIBits(hdc, hBmp, 0, bmi.bmiHeader.biHeight, arr, bi, DIB_RGB_COLORS);
	err = GetLastError();
	if (0 == res)
		throw "Cannot manipulate target bitmap";

	BITMAPFILEHEADER bmfh;
	bmfh.bfType = 0x4D42;
	bmfh.bfSize = 
		sizeof(BITMAPFILEHEADER)+
		sizeof(BITMAPINFOHEADER)+
		256*sizeof(RGBQUAD)+
		pBI->bmiHeader.biSizeImage;
	bmfh.bfReserved1 = bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = 
		sizeof(BITMAPFILEHEADER)+
		sizeof(BITMAPINFOHEADER)+
		256*sizeof(RGBQUAD);

	fwrite(&bmfh, sizeof(BITMAPFILEHEADER), 1, trg);

	fwrite(pBI, sizeof(BITMAPINFOHEADER), 1, trg);
	fwrite(pBI->bmiColors, sizeof(RGBQUAD), 256, trg);

	fwrite(arr, pBI->bmiHeader.biSizeImage, 1, trg);
}

int main(int argc, char** argv)
{
	if (argc != 4)
	{
		printf("Syntax is: %s bmpin scriptfile bmpout\n", argv[0]);
		return -1;
	}

	try
	{
		puts("Reading source bitmap...\n");
		Bitmap bmp(argv[1]);
		MemoryDC mdc(bmp);
		puts("Reading script file...\n");
		SrcFile src(argv[2]);
		SIZE sz = bmp.Dim();
		puts("Executing script...\n");
		Execute(src, mdc, sz);
		puts("Saving output image...\n");
		SaveOutput(argv[3], sz, bmp, mdc);
		puts("Full success!!!\n");
	}
	catch (char* txt)
	{
		puts("Error: ");
		puts(txt);
		putchar('\n');
		return 1;
	}

	return 0;
}
