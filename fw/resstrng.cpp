#include <cassert>
#include <string>

#include <windows.h>

#include "resstrng.h"
#include "exception.h"
#include "window.h"
#include "menu.h"
#include "mdimenus.h"
#include "app.h"

ResString::ResString(size_t sz, UINT id) : _initSize(sz)
{
	reserve(_initSize+1);
	resize(_initSize);
	size_t n = LoadString(App::GetInstance(), id, &operator[](0), capacity());
	resize(n);
}


void ResString::Load(UINT id)
{
	reserve(_initSize+1);
	resize(_initSize);
	size_t n = LoadString(App::GetInstance(), id, &operator[](0), capacity());
	resize(n);
}
