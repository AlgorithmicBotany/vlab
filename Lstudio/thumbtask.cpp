#include <fw.h>

#include "thumbtask.h"
#include "linethcb.h"
#include "linethumb.h"

void ThumbTask::ButtonDown(int x, int y)
{
	SetCapture(_pThumb->Hwnd());
	_LastPos.x = x;
	_LastPos.y = y;
}

void MovingTask::ButtonDown(int x, int y)
{
	ThumbTask::ButtonDown(x, y);
	_pThumb->MovePointTo(x);
}


void MovingTask::MouseMove(int x, int y)
{
	_pThumb->MovePoint(x - _LastPos.x);
	_LastPos.x = x;
	_LastPos.y = y;
	if (x<0)
	{
		_pThumb->SetShift(-x);
		if (0 == _TimerId)
			_TimerId = static_cast<int>(SetTimer(_pThumb->Hwnd(), LineThumb::eScrollDown, 50, 0));
	}
	else if (x>=_pThumb->Width())
	{
		_pThumb->SetShift(x-_pThumb->Width());
		if (0 == _TimerId)
            _TimerId = static_cast<int>(SetTimer(_pThumb->Hwnd(), LineThumb::eScrollUp, 50, 0));
	}
	else if (0 != _TimerId)
	{
		KillTimer(_pThumb->Hwnd(), _TimerId);
		_TimerId = 0;
	}
}

void MovingTask::ButtonUp()
{
	if (_TimerId != 0)
	{
		KillTimer(_pThumb->Hwnd(), _TimerId);
		_TimerId = 0;
	}

	ThumbTask::ButtonUp();
	_pThumb->AdjustScale();
}


