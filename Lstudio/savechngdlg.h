#ifndef __SAVECHANGESDLG_H__
#define __SAVECHANGESDLG_H__


class SaveChangesDlg : public Dialog
{
public:
	SaveChangesDlg(const char*, const DiffList&);
	bool Command(int, Window, UINT);
	bool DoInit();
private:
	std::string _obj;
	const DiffList& _dl;

	RECT _DlgER;
	Window _Yes;
	RECT _YesER;
	Window _No;
	RECT _NoER;
	Window _Cancel;
	RECT _CancelER;
	Window _Details;
	RECT _DetailsER;
	ListView _Diff;
	Window _DiffLbl;
	int _Shift;

	bool _collapsed;
	void _Collapse();
	void _Expand();
	void _ShowDetails();
};

#else
	#error File already included
#endif
