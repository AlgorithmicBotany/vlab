#include <fw.h>

#include "linethcb.h"
#include "surfthumbcb.h"

#include "objfgvedit.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"

#include "surfaceedit.h"


void XMovedCallback::Moved(float x, bool final)
{
	_pEdit->MoveX(x, final);
}

void YMovedCallback::Moved(float y, bool final)
{
	_pEdit->MoveY(y, final);
}

void ZMovedCallback::Moved(float z, bool final)
{
	_pEdit->MoveZ(z, final);
}
