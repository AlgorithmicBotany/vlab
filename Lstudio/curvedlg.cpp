#include <fw.h>

#include "resource.h"
#include "curvedlg.h"


CurveTurtleDlg::CurveTurtleDlg() : Dialog(IDD_CRV_TURTLE)
{}


void CurveTurtleDlg::UpdateData(bool which)
{
	DX(_contactX, IDC_CRV_TURTLE_CX, which);
	DX(_contactY, IDC_CRV_TURTLE_CY, which);
	DX(_contactZ, IDC_CRV_TURTLE_CZ, which);
	DX(_endX, IDC_CRV_TURTLE_EX, which);
	DX(_endY, IDC_CRV_TURTLE_EY, which);
	DX(_endZ, IDC_CRV_TURTLE_EZ, which);
	DX(_headX, IDC_CRV_TURTLE_HX, which);
	DX(_headY, IDC_CRV_TURTLE_HY, which);
	DX(_headZ, IDC_CRV_TURTLE_HZ, which);
	DX(_upX, IDC_CRV_TURTLE_UX, which);
	DX(_upY, IDC_CRV_TURTLE_UY, which);
	DX(_upZ, IDC_CRV_TURTLE_UZ, which);
	DX(_size, IDC_CRV_TURTLE_S, which);
}



CurveTransformDlg::CurveTransformDlg() : Dialog(IDD_CRV_TRANSFORM)
{}


void CurveTransformDlg::UpdateData(bool which)
{
	DX(_rotA, IDC_CRV_TRANSFORM_RA, which);
	DX(_rotX, IDC_CRV_TRANSFORM_RX, which);
	DX(_rotY, IDC_CRV_TRANSFORM_RY, which);
	DX(_rotZ, IDC_CRV_TRANSFORM_RZ, which);
	DX(_scaleX, IDC_CRV_TRANSFORM_SX, which);
	DX(_scaleY, IDC_CRV_TRANSFORM_SY, which);
	DX(_scaleZ, IDC_CRV_TRANSFORM_SZ, which);
	DX(_transX, IDC_CRV_TRANSFORM_TX, which);
	DX(_transY, IDC_CRV_TRANSFORM_TY, which);
	DX(_transZ, IDC_CRV_TRANSFORM_TZ, which);
	DX(_setX, IDC_CRV_TRANSFORM_EX, which);
	DX(_setY, IDC_CRV_TRANSFORM_EY, which);
	DX(_setZ, IDC_CRV_TRANSFORM_EZ, which);
}


bool CurveTransformDlg::Command(int id, Window, UINT)
{
	switch (id)
	{
		case IDB_CRV_TRANSFORM_R :
		case IDB_CRV_TRANSFORM_S :
		case IDB_CRV_TRANSFORM_T :
		case IDB_CRV_TRANSFORM_E :
			{
				UpdateData(true);
				EndDialog(id);
			}
			return true;
			break;
	}

	return false;
}


CurveCAGDDlg::CurveCAGDDlg() : Dialog(IDD_CRV_CAGD)
{}


void CurveCAGDDlg::UpdateData(bool which)
{
	DX(_crvA, IDC_CRV_CAGD_A, which);
	DX(_crvB, IDC_CRV_CAGD_B, which);
	DX(_crvC, IDC_CRV_CAGD_C, which);
	DX(_crvD, IDC_CRV_CAGD_D, which);
	DX(_crvF, IDC_CRV_CAGD_F, which);
	DX(_crvT, IDC_CRV_CAGD_T, which);
	DX(_crvN, IDC_CRV_CAGD_N, which);
}


