#include <cassert>

#include <string>

#include <windows.h>
#include <windowsx.h>

#include "warningset.h"

#include "menu.h"
#include "exception.h"
#include "window.h"
#include "button.h"
#include "bitmap.h"


void Button::SetIcon(const Icon& icon)
{
	SendMessage(Hwnd(), BM_SETIMAGE, IMAGE_ICON, reinterpret_cast<LPARAM>(icon.Handle()));
}
