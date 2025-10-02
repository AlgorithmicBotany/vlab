#include <warningset.h>

#include <math.h>
#include <fw.h>
#include <gl\gl.h>

#include "resource.h"

HINSTANCE hDllInstance = 0;


BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD reason, void*)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH :
		hDllInstance = hInst;
		break;
	case DLL_PROCESS_DETACH :
		break;
	}
	return true;
}
