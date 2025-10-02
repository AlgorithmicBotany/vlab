#ifndef __OPENBROWSERDIALOGS_H__
#define __OPENBROWSERDIALOGS_H__



class OpenBrowserRemoteDlg : public Dialog
{
public:
	OpenBrowserRemoteDlg
		(
		LStudioOptions&,
		const std::string&
		);
	void UpdateData(bool);
	const std::string& Host() const
	{ return _host; }
	const std::string& User() const
	{ return _user; }
	const std::string& Password() const
	{ return _pswd; }
	const std::string& Oofs() const
	{ return _oofs; }
	void SetDefaults(LStudioOptions& pref) const
	{
		pref.Host(_host.c_str());
		pref.User(_user.c_str());
		pref.Password(_pswd.c_str());
		pref.Oofs(_oofs.c_str());
	}
	bool Command(int, Window, UINT);
	bool DoInit();
private:
	bool _Check();
	void MakeDefault();
	LStudioOptions& _options;
	const std::string& _fname;
	std::string _host;
	std::string _user;
	std::string _pswd;
	std::string _oofs;
};


class OpenBrowserLocalDlg : public Dialog
{
public:
	OpenBrowserLocalDlg
		(
		LStudioOptions& options, 
		const std::string& optfname
		);
	void UpdateData(bool what);
	const std::string& Oofs() const
	{ return _oofs; }
	bool Command(int, Window, UINT);
	bool DoInit();
private:
	bool _Check();
	void BrowseOofs();
	void MakeDefault();
	LStudioOptions& _options;
	const std::string& _fname;
	std::string _oofs;
};



#endif
