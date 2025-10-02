#include <fw.h>
#include <glfw.h>


#include "setfncpntdlg.h"
#include "resource.h"


SetFuncPntDlg::SetFuncPntDlg(WorldPointf wp, float minv, float maxv) :
Dialog(IDD_INPUTFUNCPOINT),
_x(wp.X()), _y(wp.Y()), _minv(minv), _maxv(maxv)
{
}


bool SetFuncPntDlg::DoInit()
{
	const int BfSize = 64;
	TCHAR bf[BfSize];
	Window w(GetDlgItem(IDC_MINVAL));
	_stprintf(bf, __TEXT("%f<="), _minv);
	w.SetText(bf);
	w.Reset(GetDlgItem(IDC_XVAL));
	_stprintf(bf, __TEXT("%f"), _x);
	w.SetText(bf);
	w.Reset(GetDlgItem(IDC_MAXVAL));
	_stprintf(bf, __TEXT("<=%f"), _maxv);
	w.SetText(bf);
	w.Reset(GetDlgItem(IDC_YVAL));
	_stprintf(bf, __TEXT("%f"), _y);
	w.SetText(bf);
	return true;
}


void SetFuncPntDlg::UpdateData(bool what)
{
	DX(_x, IDC_XVAL, what);
	DX(_y, IDC_YVAL, what);
}


bool SetFuncPntDlg::_Check()
{
	if (_x<_minv)
	{
		_CheckFailed(IDERR_XTOOSMALL, IDC_XVAL);
		return false;
	}
	if (_x>_maxv)
	{
		_CheckFailed(IDERR_XTOOBIG, IDC_XVAL);
		return false;
	}
	return true;
}
