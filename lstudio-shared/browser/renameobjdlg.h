#ifndef __RENAMEOBJDLG_H__
#define __RENAMEOBJDLG_H__



class RenameObjDlg : public Dialog
{
public:
	RenameObjDlg(WORD dlgid, int idc_name, const char* name) : Dialog(dlgid), _idc_name(idc_name), _name(name)
	{}
	void UpdateData(bool what)
	{
		DX(_name, _idc_name, what);
	}
	const char* Name() const
	{ return _name.c_str(); }
private:
	const int _idc_name;
	std::string _name;
};



#else
	#error File already included
#endif
