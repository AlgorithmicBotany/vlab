/**************************************************************************

  File:		selectidlg.cpp
  Created:	04-Dec-97


  Implementation of class SelectIdDlg


**************************************************************************/



#include "warningset.h"

#include <windows.h>

#include "dialog.h"
#include "selectidlg.h"
#include "resource.h"
#include "dll.h"

SelectIdDlg::SelectIdDlg() : Dialog(IDD_SELECTID, hDllInstance)
{}

int SelectIdDlg::GetId()
{
	return 1;//_id;
}



