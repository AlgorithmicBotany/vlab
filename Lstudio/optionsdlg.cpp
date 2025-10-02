#include <fw.h>

#include "optionsdlg.h"
#include "lstudioptns.h"
#include "resource.h"


OptionsDlg::OptionsDlg(const LStudioOptions& opt) : 
Dialog(IDD_OPTIONS)
{
	switch (opt.GetDisplayOnTabs())
	{
	case Options::dtText :
		_showCtl = IDC_LABELS;
		break;
	case Options::dtIcons :
		_showCtl = IDC_ICONS;
		break;
	case Options::dtBoth :
		_showCtl = IDC_BOTH;
		break;
	}
}


void OptionsDlg::UpdateData(bool what)
{
	Button Txt(GetDlgItem(IDC_LABELS));
	Button Icn(GetDlgItem(IDC_ICONS));
	Button Bth(GetDlgItem(IDC_BOTH));
	if (what)
	{
		if (Txt.IsChecked())
			_showCtl = IDC_LABELS;
		else if (Icn.IsChecked())
			_showCtl = IDC_ICONS;
		else 
			_showCtl = IDC_BOTH;
	}
	else
	{
		Txt.SetCheck(false);
		Icn.SetCheck(false);
		Bth.SetCheck(false);
		switch (_showCtl)
		{
		case IDC_LABELS :
			Txt.SetCheck(true);
			break;
		case IDC_ICONS :
			Icn.SetCheck(true);
			break;
		case IDC_BOTH :
			Bth.SetCheck(true);
			break;
		}
	}
}


void OptionsDlg::UpdateOptions(LStudioOptions& options) const
{
	switch (_showCtl)
	{
	case IDC_LABELS :
		options.SetDisplayOnTabs(Options::dtText);
		break;
	case IDC_ICONS :
		options.SetDisplayOnTabs(Options::dtIcons);
		break;
	case IDC_BOTH :
		options.SetDisplayOnTabs(Options::dtBoth);
		break;
	}
}


