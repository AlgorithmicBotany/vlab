/**************************************************************************

  File:		window.cpp
  Created:	25-Nov-97


  Implementation of class Window


**************************************************************************/


#include <assert.h>
#include <stdio.h>
#include <string>


#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "lstring.h"
#include "mdimenus.h"
#include "app.h"
#include "libstrng.h"
#include "gdiobjs.h"
#include "resstrng.h"
#include "bitmap.h"

void Window::GetText(LongString& str) const
{
	int len = GetWindowTextLength(_hWnd);
	if (str._size<=len)
		str.SetSize(len+1);
	GetWindowText(_hWnd, str._arr, str._size);
	str._len = _tcslen(str._arr);
}


void Window::GetText(std::string& str) const
{
	int len = GetTextLength();
	if (len>0)
	{
		str.reserve(len+1);
		str.resize(len);
		len = GetWindowText(_hWnd, &str[0], len+1);
		str.resize(len);
	}
	else
		str = "";
}

void Window::SetRegion(const RegionBase& region) const
{ SetWindowRgn(Hwnd(), region.Handle(), TRUE); }


void Window::MessageBox(const std::string& msg) const
{
	::MessageBox(_hWnd, msg.c_str(), __TEXT("Info"), MB_ICONINFORMATION);
}


void Window::SetText(UINT id, int len) const
{
	ResString txt(len, id);
	SetText(txt);
}


void Window::SetFloat(float v) const
{
	const int BfSize = 32;
	TCHAR bf[BfSize];
	int n = _sntprintf(bf, BfSize-1, __TEXT("%f"), v)-1;
	while (__TEXT('0')==bf[n])
	{
		bf[n] = 0;
		n--;
	}
	SetWindowText(_hWnd, bf);
}


void Window::SetInt(int v) const
{
	const int BfSize = 32;
	TCHAR bf[BfSize];
	_sntprintf(bf, BfSize-1, __TEXT("%d"), v);
	SetWindowText(_hWnd, bf);
}

int Window::GetInt() const
{
	const int BfSize = 16;
	char bf[BfSize+1];
	GetWindowText(_hWnd, bf, BfSize);
	return atoi(bf);
}

void Window::MessageBox(UINT id, ...) const
{
	const int FmtLen = 128;
	ResString format(FmtLen, id);
	const int BfSize = 256;
	std::string bf;
	bf.reserve(BfSize+1);
	bf.resize(BfSize);
	int res;
	va_list args;
	va_start(args, id);
	res = _vsntprintf(&bf[0], BfSize, format.c_str(), args);
	va_end(args);
	bf.resize(res);
	::MessageBox(_hWnd, bf.c_str(), FWStr::GetLibString(FWStr::Message), MB_ICONEXCLAMATION);	
}


void Window::MessageBox(const char* fmt, ...) const
{
	const int BfSize = 256;
	std::string bf;
	bf.resize(BfSize);
	int res;
	va_list args;
	va_start(args, fmt);
	res = _vsntprintf(&bf[0], BfSize, fmt, args);
	va_end(args);
	bf.resize(res);
	::MessageBox(_hWnd, bf.c_str(), FWStr::GetLibString(FWStr::Message), MB_ICONEXCLAMATION);
}


bool Window::MessageYesNo(UINT id, ...) const
{
	const int FmtLen = 128;
	ResString format(FmtLen, id);
	const int BfSize = 256;
	std::string bf;
	bf.reserve(BfSize+1);
	bf.resize(BfSize);
	int res;
	va_list args;
	va_start(args, id);
	res = _vsntprintf(&(bf[0]), BfSize, format.c_str(), args);
	va_end(args);
	bf.resize(res);
	return (IDYES == ::MessageBox(_hWnd, bf.c_str(), FWStr::GetLibString(FWStr::YesNo), MB_ICONQUESTION | MB_YESNO));
}


bool Window::MessageYesNo(const char* fmt, ...) const
{
	const int BfSize = 256;
	std::string bf;
	bf.resize(BfSize);
	int res;
	va_list args;
	va_start(args, fmt);
	res = _vsntprintf(&(bf[0]), BfSize, fmt, args);
	va_end(args);
	bf.resize(res);
	return (IDYES == ::MessageBox(_hWnd, bf.c_str(), FWStr::GetLibString(FWStr::YesNo), MB_ICONQUESTION | MB_YESNO));
}


void Window::ErrorBox(int sz, UINT id) const
{
	ResString msg(sz, id);
	::MessageBox(_hWnd, msg.c_str(), FWStr::GetLibString(FWStr::Error), MB_ICONERROR);
}

void Window::MoveBy(int dx, int dy) const
{
	RECT r;
	GetWindowRect(r);
	::OffsetRect(&r, dx, dy);
	r.right -= r.left;
	r.bottom -= r.top;
	MoveWindow(r);
}


void Window::SetIcon(const Icon& icon) const
{
	SendMessage(_hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon.Handle()));
	SendMessage(_hWnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon.Handle()));
}


void Window::SetMenu(const Menu& menu) const
{
	::SetMenu(Hwnd(), menu.Handle());
}

void Window::SetFont(const Font& font) const
{
	SendMessage(_hWnd, WM_SETFONT, (WPARAM) font.HObj(), MAKELPARAM(true, 0)); 
}
