/**************************************************************************

  File:		lstring.cpp
  Created:	25-Nov-97


  Implementation of class LongString


**************************************************************************/


#include <cassert>
#include <cstdio>

#include <string>

#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "mdimenus.h"
#include "exception.h"
#include "menu.h"
#include "window.h"
#include "app.h"
#include "lstring.h"
#include "file.h"


LongString::LongString(int l)
{
	assert(l>1);
	_size = l;
	_arr = new TCHAR[_size];
	_len = 0;
	_arr[0] = 0;
}

LongString::LongString(int l, UINT id)
{
	assert(l>0);
	_arr = new TCHAR[l];
	_size = l;
	_len = LoadString(App::GetInstance(), id, _arr, _size);
}

LongString::LongString(int l, UINT id, HINSTANCE hInst)
{
	assert(l>0);
	assert(0 != hInst);
	_arr = new TCHAR[l];
	_size = l;
	_len = LoadString(hInst, id, _arr, _size);
}


LongString::LongString(const TCHAR* str)
{
	int l = _tcslen(str) + 1;
	_size = 128;
	while (_size < l)
		_size *= 2;
	_arr = new TCHAR[_size];
	_tcscpy(_arr, str);
	_len = l-1;
}

LongString::~LongString()
{
	delete []_arr;
}


void LongString::Load(UINT id)
{
	_len = LoadString(App::GetInstance(), id, _arr, _size);
}

void LongString::SetSize(int size)
{
	assert(size>0);
	TCHAR* aNew = new TCHAR[size];
	_size = size;
	delete []_arr;
	_arr = aNew;
	_arr[0] = 0;
	_len = 0;
}

void LongString::operator =(const TCHAR* str)
{
	int len = _tcslen(str)+1;
	int newsz = _size;
	while (newsz < len)
		newsz *= 2;
	SetSize(newsz);
	_tcscpy(_arr, str);
	_len = len-1;
}


void LongString::operator =(const LongString& src)
{
	if (src._len+1 > _size)
	{
		int newsz = _size*2;
		while (newsz<src._len+1)
			newsz *= 2;
		SetSize(newsz);
	}
	_tcscpy(_arr, src._arr);
	_len = src._len;
}

void LongString::operator +=(const TCHAR* str)
{
	int strl = _tcslen(str);
	if (0 == strl)
		return;
	int newlen = _len + strl;
	if (newlen+1>_size)
		GrowTo(newlen+1);
	_tcscpy(_arr+_len, str);
	_len = newlen;
}

void LongString::operator+=(TCHAR c)
{
	if (_len==_size-1)
		_Grow(_size*2);
	_arr[_len++] = c;
	_arr[_len] = 0;
}



void LongString::_Grow(int sz)
{
	assert(sz>_size);
	TCHAR* aNew = new TCHAR[sz];
	_size = sz;
	_tcscpy(aNew, _arr);
	delete []_arr;
	_arr = aNew;
}


void LongString::GrowTo(int sz)
{
	int newsz = _size;
	while (newsz<sz)
		newsz *= 2;
	_Grow(newsz);
}

void LongString::Write(WriteTextFile& trg) const
{
	trg.Write(_arr);
}


void LongString::ExchangeBuffers(LongString& src)
{
	TCHAR* pTmp = src._arr;
	int tmplen = src._len;
	int tmpsize = src._size;
	src._arr = _arr;
	src._len = _len;
	src._size = _size;
	_arr = pTmp;
	_len = tmplen;
	_size = tmpsize;
}


void LongString::AllTrim()
{
	int ix = 0;

	// Trim leading blanks
	while ((ix<=_len) && _istspace(_arr[ix]))
		ix++;

	if (ix>0)
	{
		memmove(_arr, _arr+ix, (_size-ix)*sizeof(TCHAR));
		_len -= ix;
	}

	ix = _len-1;
	while ((ix>=0) && _istspace(_arr[ix]))
		ix--;
	ix++;

	if (ix<_len)
	{
		_arr[ix]=0;
		_len = ix;
	}
}

void LongString::CutLast(int n)
{
	assert(n<=_len);
	assert(n>0);
	_len -= n;
	_arr[_len] = 0;
}


int LongString::CalcLength()
{
	_len = _tcslen(_arr);
	assert(_len<_size);
	return _len;
}


bool LongString::Format(const TCHAR* frmt, ...)
{
	va_list args;
	va_start(args, frmt);
	int res = _vsntprintf(_arr, _size, frmt, args);
	va_end(args);
	if (-1 == res)
	{
		_arr[0] = 0;
		_len = 0;
		return false;
	}
	_len = res;
	return true;
}


void LongString::ToUpper()
{
	_tcsupr(_arr);
}


void LongString::Pack()
{
	if (_len<128 && _size>128)
		_Grow(128);
}

