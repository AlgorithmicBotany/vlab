#include <fw.h>

#include <gl\gl.h>

#include "displists.h"
#include "fontlist.h"

void FontList::Init(HDC hdc) 
{
	_lists.Init(eFontRange);
	wglUseFontBitmaps(hdc, 0, eFontRange, _lists.Base());
}

