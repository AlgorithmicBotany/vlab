#include <string>
#include <cassert>

#include <windows.h>

#include "warningset.h"

#include "exception.h"
#include "canvas.h"
#include "window.h"

UpdateCanvas::UpdateCanvas(Window* w) : _hWnd(w->Hwnd())
{
	SetDC(::GetDC(_hWnd));
}
