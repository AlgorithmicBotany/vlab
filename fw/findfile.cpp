#include <string>
#include <cassert>

#include <tchar.h>
#include <windows.h>

#include "warningset.h"

#include "findfile.h"



FindFile::FindFile(const std::string& fname)
{
	_hFind = FindFirstFile(fname.c_str(), &_find);
	if (Found())
		_fname = _find.cFileName;
}


FindFile::~FindFile()
{
	if (INVALID_HANDLE_VALUE != _hFind)
		FindClose(_hFind);
}


bool FindFile::FindNext()
{
	assert(INVALID_HANDLE_VALUE != _hFind);
	if (FindNextFile(_hFind, &_find))
	{
		_fname = _find.cFileName;
		return true;
	}
	else
	{
		_find.cFileName[0] = 0;
		_fname.erase();
		FindClose(_hFind);
		_hFind = INVALID_HANDLE_VALUE;
		return false;
	}
}

bool FindFile::IsSubDirectory() const
{
	if (!IsDirectory())
		return false;
	if (0==FileName().compare("."))
		return false;
	if (0==FileName().compare(".."))
		return false;
	return true;
}

bool FindFile::FileExists(const std::string& fname)
{
	FindFile ff(fname);
	return ff.Found();
}

bool FindFile::StartsWith(const std::string& prefix) const
{
	if (prefix.length()>FileName().length())
		return false;
	return (0==_strnicmp(prefix.c_str(), FileName().c_str(), prefix.length()));
}

bool FindFile::EndsWith(const std::string& suffix) const
{
	if (suffix.length()>FileName().length())
		return false;
	size_t len1 = suffix.length();
	size_t len2 = FileName().length();
	if (len1>len2)
		return false;
	const char* sfx = FileName().c_str()+len2-len1;
	return (0==_strnicmp(sfx, suffix.c_str(), len1));
}

bool FindFile::FilenameIs(const std::string& name) const
{
	return (0==_stricmp(name.c_str(), FileName().c_str()));
}
