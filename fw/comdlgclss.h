/**************************************************************************

  File:		comdlgclss.h
  Created:	08-Jan-98


  Declaration of common dialogs classes


**************************************************************************/


#ifndef __COMDLGCLSS_H__
#define __COMDLGCLSS_H__


class Choosefont : public CHOOSEFONT
{
public:
	Choosefont(HWND, int, const std::string&);
	Choosefont(HWND, const LogFont&);
	BOOL Display();
	const LogFont& GetLogFont() const
	{ return _lf; }
private:
	LogFont _lf;
};


class OpenFilename : public OPENFILENAME
{
public:
	OpenFilename(HWND, UINT, const std::string&);
	void SaveSecurity()
	{
		Flags |= (OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY);
	}
	void OpenSecurity()
	{
		Flags |= (OFN_FILEMUSTEXIST);
	}
	BOOL Open()
	{ return GetOpenFileName(this); }
	BOOL Save()
	{ return GetSaveFileName(this); }
	const TCHAR* Filename() const
	{ return _Filename; }
	void SetDefault(const std::string&);
	void SetDirectory(const std::string&);
private:
	enum eParams
	{
		nMaxFilterSize = 255,
		nMaxExtSize = 16,
		nMaxTitleSize = 256
	};
	TCHAR _Filter[nMaxFilterSize+1];
	TCHAR _DefExt[nMaxExtSize+1];
	TCHAR _Filename[_MAX_PATH+1];
	TCHAR _FileTitle[nMaxTitleSize+1];
	TCHAR _InitDir[_MAX_PATH+1];
};



class Choosecolor : public CHOOSECOLOR
{
public:
	Choosecolor(HWND, COLORREF);
	BOOL Choose();
private:
	COLORREF _CustColors[16];
};


#endif
