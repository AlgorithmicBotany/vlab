#ifndef __REMOTECNCT_H__
#define __REMOTECNCT_H__


namespace VLB
{

class RemoteConnection : public Connection
{
public:
	RemoteConnection(const std::string&, const std::string&, const std::string&, const string_buffer&);
	~RemoteConnection();
	bool HasExtensions(const std::string&);
	bool GetExtensions(string_buffer&, string_buffer&, std::vector<int>&, const std::string&);
	bool GetFileList(string_buffer&, const std::string&, bool);
	bool RenameObj(const std::string&, std::string&, const std::string&);
	bool FetchObject(const std::string&);
	bool PutObject(const string_buffer&);
	bool GetFile(const std::string&);
	bool GetFile(const std::string&, const std::string&);
	bool GetTmpFile(const std::string&, const std::string&, std::string&);
	bool AddId(const std::string&, const std::string&);
	bool DeleteIds(const std::string& /* path */, const std::string& /* oofs root */);
	const std::string& Host() const
	{ return _host; }
	bool PutFile(const std::string&, const std::string&);
	bool PutFile(const std::string& /* remote path */, const std::string& /* local name */, const std::string& /* remote name */);
	bool MakeExt(const std::string&, const std::string&, std::string&);
	bool DelTree(const std::string&);
	bool PrototypeObject(const std::string&);
	bool CompareFiles(const std::string&, const std::string&);
	bool DeleteFile(const std::string&, const std::string&);
	bool RequiresIds() const
	{ return true; }
	bool SupportsCopyPaste() const
	{ return true; }
	bool Paste(const std::string&, const std::string&, const std::string&, bool);
	void AppendFName(const std::string& path, const std::string& name, std::string& outputpath)
	{
		outputpath = path;
		outputpath += '/';
		outputpath += name;
	}
	char PathSeparator() const
	{ return '/'; }
private:
	bool _RetrieveMultipleStrings(string_buffer&, const char*);
	bool _RetrieveMultipleFiles(const char*);
	int _RetrieveInt(const unsigned char*);
	int _RetrieveIntRev(const unsigned char*);
	void _BuildIntRev(int, unsigned char*);
	bool _Login(const std::string&, const std::string&);
	bool _MakeDir(const std::string&);
	void _ValidateFName(std::string&) const;
	bool _ValidFname(const std::string&) const;
	const std::string _host;
	SocketUser _su;
	Socket _socket;

	bool ConvertCRLF(const std::string&) const;
	string_buffer _crlf;
};

}


#else
	#error File already included
#endif
