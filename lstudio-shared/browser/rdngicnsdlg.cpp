
#include <vector>
#include <string>

#include <process.h>
#include <fw.h>

#include "racomm.h"
#include "socket.h"
#include "connection.h"
#include "node.h"
#include "rdngicnsdlg.h"

VLB::ReadingIcnsDlg::ReadingIcnsDlg(Canvas& cnv, Connection* pConnection, Node* pParent, int width, WORD dlgid, int progress_id) : 
Dialog(dlgid),
_progress_id(progress_id),
_cnv(cnv), _pConnection(pConnection), _pParent(pParent), 
_abort(0),
_width(width)
{
}


bool VLB::ReadingIcnsDlg::DoInit()
{
	int count = _pParent->CountChildren(true) + 1;
	_Progress = GetDlgItem(_progress_id);
	_Progress.SetRange(0, count);
	_Progress.SetStep(1);
	_beginthread(_ThreadFunc, 0, this);
	return true;
}


void VLB::ReadingIcnsDlg::_DoThread()
{
	_pParent->ShowIcon(true, _pConnection, _cnv, true, &_abort, _Progress, _width);
	FORWARD_WM_COMMAND(Hdlg(), IDOK, 0, 0, PostMessage);
}


bool VLB::ReadingIcnsDlg::Command(int id, Window, UINT)
{
	if (IDCANCEL == id)
	{
		InterlockedIncrement(&_abort);
		return true;
	}
	return false;
}


