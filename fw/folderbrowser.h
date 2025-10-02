#ifndef __FOLDERBROWSER_H__
#define __FOLDERBROWSER_H__



class FolderBrowser : public SShellPtr<ITEMIDLIST>
{
public:
	FolderBrowser(HWND, UINT, UINT);
	void AllowNewFolder(bool allow)
	{
		if (allow)
			_bi.ulFlags |= BIF_NEWDIALOGSTYLE;
		else
			_bi.ulFlags &= ~(BIF_NEWDIALOGSTYLE);
	}
	bool Browse(std::string&);
private:
	BROWSEINFO _bi;
	const ResString _caption;
	const ResString _title;
	std::string _DisplayName;
	std::string _InitSel;

	static int CALLBACK _BFF(HWND, UINT, LPARAM, LPARAM);
	int _DoBFF(HWND, UINT, LPARAM);

	FolderBrowser(const FolderBrowser&);
};

class SpecialFolder : public SShellPtr<ITEMIDLIST>
{
public:
	SpecialFolder(int id, HWND hOwner = 0);
	void GetPath(std::string&);
	void GetPath(TCHAR* txt)
	{
		SHGetPathFromIDList(_p, txt);
	}
private:
	SpecialFolder(const SpecialFolder&);
};


#endif
