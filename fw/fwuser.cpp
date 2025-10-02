#include <cassert>
#include <cmath>
#include <string>

#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "exception.h"
#include "window.h"

#include "fwuser.h"

#include "menu.h"
#include "keystate.h"
#include "sizestate.h"
#include "ownerdraw.h"
#include "ctrl.h"
#include "canvas.h"
#include "colorslider.h"
#include "colors.h"
#include "huecircle.h"
#include "winmaker.h"
#include "genctrl.h"

bool FWUser::_exists = false;

FWUser::FWUser(HINSTANCE hInst) : _hInst(hInst)
{
	assert(!_exists);
	_exists = true;
	Register(_hInst);
}


FWUser::~FWUser()
{
	assert(_exists);
	_exists = false;
	Unregister(_hInst);
}

void FWUser::Register(HINSTANCE hInst)
{
	ColorSlider::Register(hInst);
	HueCircle::Register(hInst);
	GenCtrl::Register(hInst);
}


void FWUser::Unregister(HINSTANCE hInst)
{
	ColorSlider::Unregister(hInst);
	HueCircle::Unregister(hInst);
	GenCtrl::Unregister(hInst);
}



int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int show)
{
	FWUser fwu(hInst);
	return Main(hInst, static_cast<Window::sw>(show));
}
