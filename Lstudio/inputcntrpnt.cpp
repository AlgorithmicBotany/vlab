#include <fw.h>
#include <glfw.h>

#include "resource.h"
#include "inputcntrpnt.h"

void SetCntrPntDlg::UpdateData(bool what)
{
	DX(_x, IDC_EDITX, what);
	DX(_y, IDC_EDITY, what);
}

