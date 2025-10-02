#include <fw.h>

#include "aboutdlg.h"

#include "resource.h"


AboutDlg::AboutDlg() : Dialog(IDD_ABOUT)
{}


bool AboutDlg::DoInit()
{
	TextResource crdts(IDR_CREDITS, "TEXT");
	
	
	Window Wstatic(GetDlgItem(IDC_CREDITS));
	Wstatic.SetText(crdts.Text());
	return true;
}
