#include <string>
#include <cassert>

#include <windows.h>
#include <commctrl.h>

#include "warningset.h"

#include "imagelist.h"
#include "bitmap.h"

void ImageList::Build(HINSTANCE hInst, const UINT* arr, int count)
{
	for (int i=0; i<count; i+=2)
	{
		Bitmap bmp(hInst, MAKEINTRESOURCE(arr[i]));
		Bitmap bmpmask(hInst, MAKEINTRESOURCE(arr[i+1]));
		ImageList_Add(_hList, bmp.Handle(), bmpmask.Handle());
	}
}

