#include <fw.h>

#include "waitsmphrdlg.h"
#include "resource.h"

const size_t DefaultTitleLength = 64;

WaitForSemaphoreDlg::WaitForSemaphoreDlg(Semaphore& semaphore, UINT caption) :
Dialog(IDD_WAITFORSEMAPHORE),
_semaphore(semaphore),
_caption(DefaultTitleLength, caption)
{}


bool WaitForSemaphoreDlg::DoInit()
{
	_time = eDefaultWaitForFinal*2;
	SetDlgItemInt(Hdlg(), IDC_WAITING, eDefaultWaitForFinal, true);
    _timerId = static_cast<int>(SetTimer(Hdlg(), 1, 500, 0));
	SetCaption(_caption);
	return true;
}



bool WaitForSemaphoreDlg::HandleMsg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM)
{
	switch (msg)
	{
	case WM_TIMER:
		HANDLE_WM_TIMER(hDlg, wParam, lParam, Timer);
		break;
	default :
		return false;
	}
	return true;
}


void WaitForSemaphoreDlg::Timer(HWND, UINT id)
{
	if (id == _timerId)
	{
		if (_semaphore.Wait(0))
		{
			EndDialog(IDOK);
		}
		else
		{
			_time++;
			if (0==_time%2)
				SetDlgItemInt(Hdlg(), IDC_WAITING, _time/2, true);
		}
	}
}
