#include <string>

#include <cassert>
#include <windows.h>

#include "warningset.h"

#include "tmpfile.h"
#include "exception.h"
#include "tmpdir.h"

#include "libstrng.h"


TempFileName::TempFileName(const std::string& prefix)
{
	TempPath tp;
	resize(MAX_PATH+1);
	if (0==::GetTempFileName(tp.c_str(), prefix.c_str(), 0, &(at(0))))
		throw Exception(0, FWStr::ErrCreateTmpFile);
	size_t null_t = find_first_of('\0');
	resize(null_t);
}


TmpFile::TmpFile(bool bOpen)
{
	TempFileName tfname("tmp");
	_fname = tfname;
	if (bOpen)
	{
		_hFile = CreateFile
			(
			_fname.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			0,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_TEMPORARY,
			0
			);
		if (INVALID_HANDLE_VALUE == _hFile)
			throw Exception(0, FWStr::OpenFile, _fname);
	}
	else
	{
		_hFile = INVALID_HANDLE_VALUE;
	}
	_eof = false;
}

TmpFile::TmpFile(const TCHAR* fname, DWORD access, DWORD share, DWORD crt)
{
	_fname = fname;
	_hFile = CreateFile
		(
		_fname.c_str(),
		access,
		share,
		0,
		crt,
		FILE_ATTRIBUTE_TEMPORARY,
		0
		);

	if (INVALID_HANDLE_VALUE == _hFile)
		throw Exception(0, FWStr::OpenFile, fname);
	_eof = false;
}


TmpFile::~TmpFile()
{
	if (IsOpen())
	{
		::CloseHandle(_hFile);
		_hFile = INVALID_HANDLE_VALUE;
	}
	::DeleteFile(_fname.c_str());
}


void TmpFile::Write(const void* p, DWORD size)
{
	assert(IsOpen());
	DWORD aux;
	::WriteFile(_hFile, p, size, &aux, 0);
}


void TmpFile::Read(void* p, DWORD size)
{
	assert(IsOpen());
	DWORD aux;
	BOOL res = ::ReadFile(_hFile, p, size, &aux, 0);
	if ((res) && (0==aux))
		_eof = true;
}


void TmpFile::Reset()
{
	assert(IsOpen());
	::SetFilePointer(_hFile, 0, 0, FILE_BEGIN);
	_eof = false;
}


int TmpFile::Size() const
{
	assert(IsOpen());
	return ::GetFileSize(_hFile, 0);
}
