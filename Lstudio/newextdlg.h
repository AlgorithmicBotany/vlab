#ifndef __NEWEXTENSIONDLG_H__
#define __NEWEXTENSIONDLG_H__


class NewExtensionDlg : public Dialog
{
public:
	NewExtensionDlg(const std::string& path, char, bool);
	bool DoInit();
	void UpdateData(bool);
	const char* Name() const
	{ return _Name.c_str(); }
	bool ChangeIdentity() const
	{ return _ChangeIdentity; }
	bool Command(int, Window, UINT);
	bool StoreLocally() const
	{ return _StoreLocally; }
	const char* Path() const
	{ return _Location.c_str(); }
private:
	bool _Check();
	void _BrowseLocation();
	std::string _Location;
	std::string _Name;
	bool _ChangeIdentity;
	bool _StoreLocally;
};


#else
	#error File already included
#endif
