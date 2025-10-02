#ifndef __RENAMEDIALOG_H__
#define __RENAMEDIALOG_H__


class RenameDialog : public Dialog
{
public:
	RenameDialog(UINT, const TCHAR*);
	bool DoInit();
	void UpdateData(bool);
	const TCHAR* GetName() const
	{ return _name; }
private:
	const UINT _titleId;
	bool _Check();
	LongString _name;
};


#else
	#error File already included
#endif
