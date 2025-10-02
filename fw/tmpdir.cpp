#include <string>
#include <cassert>

#include <windows.h>

#include "warningset.h"

#include "tmpdir.h"
#include "exception.h"

#include "libstrng.h"


TmpChangeDir::TmpChangeDir()
{
	TempPath tmp;
	CurrentDirectory cd;
	_Original = cd;

	if (!::SetCurrentDirectory(tmp.c_str()))
		throw Exception(0, FWStr::ChDir, tmp.c_str());
}


TmpChangeDir::TmpChangeDir(const std::string& dir)
{
	CurrentDirectory cd;
	_Original = cd;

	if (!::SetCurrentDirectory(dir.c_str()))
		throw Exception(0, FWStr::ChDir, dir.c_str());
}
