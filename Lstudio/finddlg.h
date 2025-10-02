#ifndef __FINDDIALOG_H__
#define __FINDDIALOG_H__


class FindDialog : public Dialog
{
public:
	FindDialog(const std::string&, bool, EditLine);
	bool DoInit();
	void UpdateData(bool);
	const std::string& FindPattern() const
	{ return _Pattern; }
	bool Command(int, Window, UINT);
	bool MatchCase() const
	{ return _MatchCase == BST_CHECKED; }
	int StartSel() const
	{ return _startsel; }
	int EndSel() const
	{ return _endsel; }
	bool JustFind();
protected:
	EditLine _Edit;
	std::string _Pattern;
	bool _MatchCase;
private:
	bool _Find();
	void ToUpper(std::string&) const;
	enum 
	{
		eMaxLineLength = 2048
	};
	DWORD _startsel;
	DWORD _endsel;
};


#else
	#error File already included
#endif
