#include <memory>

#include <fw.h>

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"

#include "prjnotifysnk.h"
#include "contmodedit.h"


ContModeEdit::ContModeEdit(HWND hwnd, PrjNotifySink* pNotifySink) :
FormCtrl(hwnd),
_pNotifySink(pNotifySink),
_timer(this, tTimerId, SyncTimerTimeout, false)
{
	_InSync = true;
}

bool ContModeEdit::Timer(UINT)
{
	if (!_InSync)
	{
		_InSync = _ObjectModified(false);
		if (_InSync)
			_timer.Kill();
	}
	else
		_timer.Kill();
	return true;
}

void ContModeEdit::Modified(bool final)
{
	if (_pNotifySink->ContinuousMode())
	{
		ObjectEdit::ApplyNow();
		_InSync = _ObjectModified(final);
		if (_InSync)
			_timer.Kill();
		else 
			_timer.Start();
	}
}

