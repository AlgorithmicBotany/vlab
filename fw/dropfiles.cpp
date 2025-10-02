#include <string>
#include <cassert>

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>

#include "warningset.h"

#include "dropfiles.h"
#include "lstring.h"


void DroppedFiles::GetFilename(UINT id, std::string& str)
{
	size_t res = DragQueryFile(_hDrop, id, 0, 0);
	str.reserve(res+1);
	str.resize(res);
	DragQueryFile(_hDrop, id, &(str[0]), res+1);
	str.resize(res);
}
