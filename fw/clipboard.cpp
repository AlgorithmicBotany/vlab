#include <windows.h>

#include "warningset.h"

#include "exception.h"
#include "clipboard.h"

#include "libstrng.h"


UseClipboard::UseClipboard(HWND hwnd)
{
	BOOL res = OpenClipboard(hwnd);
	if (0 == res)
		throw Exception(0, FWStr::AccessClipboard);
}


ClipboardMemory::ClipboardMemory(DWORD size)
{
	_hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, size);
	if (0 == _hMem)
		throw Exception(0, FWStr::MemForClipboard);
}


ClipboardMemory::~ClipboardMemory()
{
	if (0 != _hMem)
		GlobalFree(_hMem);
}

