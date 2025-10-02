#include <vector>
#include <string>

#include <fw.h>
#include <process.h>

#include "racomm.h"
#include "socket.h"
#include "connection.h"
#include "node.h"
#include "rdngobjsdlg.h"


VLB::ReadingObjsDlg::ReadingObjsDlg(Canvas& cnv, Connection* pConnection, Node* pParent, WORD dlgid, int path_id) : 
Dialog(dlgid),
_path_id(path_id), _cnv(cnv), 
_pConnection(pConnection), _pParent(pParent), 
_abort(0)
{
}


bool VLB::ReadingObjsDlg::DoInit()
{
	_Text = GetDlgItem(_path_id);
	_beginthread(_ThreadFunc, 0, this);
	return true;
}


void VLB::ReadingObjsDlg::_DoThread()
{
	_pParent->ToggleExpand(_cnv, _pConnection, true, &_abort, _Text);
	FORWARD_WM_COMMAND(Hdlg(), IDOK, 0, 0, PostMessage);
}


bool VLB::ReadingObjsDlg::Command(int id, Window, UINT)
{
	if (IDCANCEL == id)
	{
		InterlockedIncrement(&_abort);
		return true;
	}
	return false;
}

