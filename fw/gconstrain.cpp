#include <string>

#include <cassert>

#include <windows.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "gconstrain.h"

void GeometryConstrain::SetLeft(const RECT& r)
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aLeft;
	_arr[_set].Value = ref.left - r.left;
	_set++;
}

void GeometryConstrain::SetTop(const RECT& r)
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aTop;
	_arr[_set].Value = ref.top - r.top;
	_set++;
}

void GeometryConstrain::SetMinTop(const RECT& r)
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aMinTop;
	_arr[_set].Value = ref.top - r.top;
	_set++;
}

void GeometryConstrain::SetMinLeft(const RECT& r)
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aMinLeft;
	_arr[_set].Value = ref.left - r.left;
	_set++;
}

void GeometryConstrain::SetRight(const RECT& r)
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aRight;
	_arr[_set].Value = r.right - ref.right;
	_set++;
}

void GeometryConstrain::SetBottom(const RECT& r)
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aBottom;
	_arr[_set].Value = r.bottom - ref.bottom;
	_set++;
}

void GeometryConstrain::SetHeight()
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aHeight;
	_arr[_set].Value = ref.bottom - ref.top;
	_set++;
}

void GeometryConstrain::SetWidth()
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aWidth;
	_arr[_set].Value = ref.right - ref.left;
	_set++;
}

void GeometryConstrain::SetMinWidth()
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aMinWidth;
	_arr[_set].Value = ref.right - ref.left;
	_set++;
}

void GeometryConstrain::SetMinHeight()
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aMinHeight;
	_arr[_set].Value = ref.bottom - ref.top;
	_set++;
}

void GeometryConstrain::SetPropWidth(const RECT& r)
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	float rw = static_cast<float>(ref.right-ref.left)/static_cast<float>(r.right-r.left);
	_arr[_set].Action = aPropWidth;
	_arr[_set].Value = static_cast<int>(rw*10000);
	_set++;
}

void GeometryConstrain::SetPropLeft(const RECT& r)
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	float pl = static_cast<float>(ref.left-r.left)/static_cast<float>(r.right-r.left);
	_arr[_set].Action = aPropLeft;
	_arr[_set].Value = static_cast<int>(pl*10000);
	_set++;
}


void GeometryConstrain::SetMaxLeft(const RECT& r)
{
	assert(_set<nSize);
	RECT ref;
	_w.GetWindowRect(ref);
	_arr[_set].Action = aMaxLeft;
	_arr[_set].Value = ref.left - r.left;
	_set++;
}


void GeometryConstrain::Adjust(int w, int h)
{
	RECT r = {-1, -1, -1, -1};
	for (int i=0; i<_set; i++)
	{
		switch (_arr[i].Action)
		{
		case aLeft :
			r.left = _arr[i].Value;
			break;
		case aWidth :
			if (-1 == r.right)
				r.right = r.left + _arr[i].Value;
			else
				r.left = r.right - _arr[i].Value;
			break;
		case aTop :
			r.top = _arr[i].Value;
			break;
		case aHeight :
			if (-1 == r.bottom)
				r.bottom = r.top + _arr[i].Value;
			else
				r.top = r.bottom - _arr[i].Value;
			break;
		case aRight :
			r.right = w - _arr[i].Value;
			break;
		case aBottom :
			r.bottom = h - _arr[i].Value;
			break;
		case aMinWidth :
			if (r.right - r.left < _arr[i].Value)
				r.right = r.left + _arr[i].Value;
			break;
		case aMinHeight :
			if (r.bottom - r.top < _arr[i].Value)
				r.bottom = r.top + _arr[i].Value;
			break;
		case aMinTop :
			if (r.top < _arr[i].Value)
			{
				int dy = _arr[i].Value - r.top;
				r.top += dy;
				r.bottom += dy;
			}
			break;
		case aMinLeft :
			if (r.left < _arr[i].Value)
			{
				int dx = _arr[i].Value - r.left;
				r.left += dx;
				r.right += dx;
			}
			break;
		case aPropWidth :
			if (-1 != r.left)
				r.right = r.left + int(w * _arr[i].Value / 10000.0);
			else
				r.left = r.right - int(w * _arr[i].Value / 10000.0);
			break;
		case aMaxLeft :
			if (r.left<_arr[i].Value)
				r.left = _arr[i].Value;
			break;
		case aPropLeft :
			r.left = int (w * _arr[i].Value / 10000.0);
			break;
		case aLast :
		default:
			assert(!"Unhandled action");
		}
	}
	r.bottom -= r.top;
	r.right -= r.left;
	_w.MoveWindow(r);
}
