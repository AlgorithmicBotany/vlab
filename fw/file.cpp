/**************************************************************************

  File:		file.cpp
  Created:	11-Dec-97


  Implementation of file classes


**************************************************************************/


#include <cassert>

#include <cstdio>
#include <cstdlib>

#include <string>

#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "lstring.h"
#include "exception.h"
#include "file.h"

#include "libstrng.h"



File::File(const std::string& fname, const std::string& mode): _fname(fname)
{
	_fp = _tfopen(fname.c_str(), mode.c_str());
	if (0 == _fp)
		throw Exception(0, FWStr::OpenFile, fname.c_str());
}

File::~File()
{
	fclose(_fp);
}


void ReadBinFile::Read(void* ptr, size_t size, size_t items)
{
	assert(0 != ptr);
	assert(size>0);
	assert(items>0);
	size_t ret = fread(ptr, size, items, _Fp());
	if (ret != items)
		throw Exception(0, FWStr::ReadFromFile, Filename());
}


long ReadBinFile::Size()
{
	long current = ftell(_Fp());
	fseek(_Fp(), 0, SEEK_END);
	long toret = ftell(_Fp());
	fseek(_Fp(), current, SEEK_SET);
	return toret;
}


void WriteBinFile::Write(const void* ptr, size_t size, size_t items)
{
	assert(0 != ptr);
	assert(size>0);
	assert(items>0);
	size_t ret = fwrite(ptr, size, items, _Fp());
	if (ret != items)
		throw Exception(0, FWStr::WriteToFile, Filename());
}



/*
void ReadTextFile::Read(char* txt, int maxlen)
{
	if (_Eof)
		throw Exception(0,FWStr::UnexpectedEOF, Filename());
	const char* res = fgets(txt, maxlen, _Fp());
	if (0 == res && feof(_Fp()))
	{
		_Eof = true;
		txt[0] = 0;
	}
	int l = strlen(txt);
	if (l>0)
	{
		if (txt[l-1]=='\n')
		{
			_line++;
			txt[l-1] = 0;
		}
		else if (l==maxlen-1)
			throw Exception(0,FWStr::LineTooLong, _line+1, Filename());
		else
			_Eof = true;
	}
}
*/

void ReadTextFile::Read(std::string& str)
{
	str.erase();
	if (_Eof)
		throw Exception(0, FWStr::UnexpectedEOF, Filename());
	for (;;)
	{
		int c = fgetc(_Fp());
		if (EOF == c)
		{
			_Eof = true;
			break;
		}
		else if ('\n' == c)
			break;
		else
			str.append(1, static_cast<char>(c));
	}
}

char ReadTextFile::GetChar()
{
	if (_Eof)
		throw Exception(0,FWStr::UnexpectedEOF, Filename());
	int c = fgetc(_Fp());
	if (EOF == c)
		_Eof = true;
	if ('\n' == c)
		_line++;

	return static_cast<char>(c);
}

void WriteTextFile::Write(const char* bf)
{
	if (EOF == fputs(bf, _Fp()))
		throw Exception(0,FWStr::WriteToFile, Filename());
}

void WriteTextFile::Write(const char* bf, int sz)
{
	for (int i=0; i<sz; ++i)
	{
		if (EOF == fputc(bf[i], _Fp()))
			throw Exception(0,FWStr::WriteToFile, Filename());
	}
}

void WriteTextFile::Write(char z)
{
	if (EOF == fputc(z, _Fp()))
		throw Exception(0,FWStr::WriteToFile, Filename());
}


void WriteTextFile::WriteLn(const char* bf)
{
	Write(bf);
	WriteEOL();
}

void WriteTextFile::WriteEOL()
{
	if (EOF == fputc('\n', _Fp()))
		throw Exception(0,FWStr::WriteToFile, Filename());
}


void WriteTextFile::PrintF(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	int res = vfprintf(_Fp(), format, args);
	va_end(args);
	if (res<0)
		throw Exception(0,FWStr::WriteToFile, Filename());
}



