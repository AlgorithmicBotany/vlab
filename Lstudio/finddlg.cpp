#include <fw.h>

#include "finddlg.h"
#include "resource.h"

FindDialog::FindDialog(const std::string& pattern, bool MatchCase, EditLine Edit) :
Dialog(IDD_FIND),
_Edit(Edit),
_Pattern(pattern),
_MatchCase(MatchCase)
{}

bool FindDialog::DoInit()
{
	return false;
}

void FindDialog::UpdateData(bool which)
{
	DX(_Pattern, IDC_FIND, which);
	DXButton(_MatchCase, IDC_MATCHCASE, which);
}


bool FindDialog::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_FIND :
			UpdateData(true);
			if (_Pattern.length()>0)
			{
				EndDialog(_Find() ? IDOK : -1);
			}
			break;
		default :
			return false;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


bool FindDialog::_Find()
{
	if (!(BST_CHECKED == _MatchCase))
		ToUpper(_Pattern);

	std::string linebf; 
	DWORD startsel, endsel;
	_Edit.GetSel(&startsel, &endsel);
	int line = _Edit.LineFromChar(endsel);
	const int FirstCheckedLine = line;
	const int LineCount = _Edit.GetLineCount();

	// current line first
	int LineIdx = _Edit.LineIndex(line);
	assert(int(endsel)>=LineIdx);
	_Edit.GetLine(line, linebf);
	if (!(BST_CHECKED == _MatchCase))
		ToUpper(linebf);
	const char* aFnd = _tcsstr(linebf.c_str()+int(endsel)-LineIdx, _Pattern.c_str());

	while (0 == aFnd)
	{
		line++;
		if (line==LineCount)
			line = 0;
		LineIdx = _Edit.LineIndex(line);
		_Edit.GetLine(line, linebf);
		if (!(BST_CHECKED==_MatchCase))
			ToUpper(linebf);
		aFnd = _tcsstr(linebf.c_str(), _Pattern.c_str());
		if (line==FirstCheckedLine && 0 == aFnd)
			break;
	}
	if (0 != aFnd)
	{
		_startsel = static_cast<DWORD>(LineIdx + static_cast<ptrdiff_t>(aFnd - linebf.c_str()));
		_endsel = _startsel + _Pattern.length();
		if (endsel == _endsel && startsel == _startsel)
			aFnd = 0;
	}
	return (0 != aFnd);
}


bool FindDialog::JustFind()
{
	return _Find();
}


void FindDialog::ToUpper(std::string& str) const
{
	std::string::const_iterator cit = str.begin();
	std::string::iterator it = str.begin();
	while (cit != str.end())
	{
		*it = static_cast<char>(toupper(*cit));
		++it; ++cit;
	}
}
