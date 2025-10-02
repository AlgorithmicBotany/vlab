#ifndef __ANYTEXTEDIT_H__
#define __ANYTEXTEDIT_H__


class EmptyName
{
public:
	static const char* Name()
	{ return ""; }
};

class AnyTextEdit : public TextEdit<EmptyName, IDD_ANYTEXT>
{
public:
	AnyTextEdit(HWND, PrjNotifySink*);
	~AnyTextEdit();

	static HWND Create(HWND, HINSTANCE, PrjNotifySink*);

	// Message handlers
	bool Command(int, Window, UINT);

	bool ShouldSave() const;
	void Save();
	const TCHAR* ShortFName() const;
private:
	const TCHAR* _DefaultName() const
	{ return __TEXT(""); }

	void _Load();
	void _SaveAs();
	void _Close();
	void _Reload();

	PrjNotifySink* _pNotifySink;

	Icon _iOpen;
	Icon _iClose;
	Icon _iSave;
	Icon _iSaveAs;
};



#else
	#error File already included
#endif
