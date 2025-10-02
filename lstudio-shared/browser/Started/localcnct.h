#ifndef __LOCALCNCT_H__
#define __LOCALCNCT_H__

namespace VLB
{

class LocalConnection : public Connection
{
public:
	LocalConnection();
	~LocalConnection();
	bool HasExtensions(const std::string&);
	bool GetExtensions(string_buffer&, string_buffer&, std::vector<int>&, const std::string&);
	bool GetFileList(string_buffer&, const std::string&, bool);
	bool RenameObj(const std::string&, std::string&, const std::string&);
	bool FetchObject(const std::string&); 
	bool PutObject(const string_buffer&);
	bool GetFile(const std::string&);
	bool GetFile(const std::string&, const std::string&);
	bool GetTmpFile(const std::string&, const std::string&, std::string&);
	bool AddId(const std::string&, const std::string&) 
	{ return true; }
	bool DeleteIds(const std::string& /* path */, const std::string& /* oofs root */)
	{ return true; }
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
	{ return false; }
	bool SupportsCopyPaste() const
	{ return true; }
	bool Paste(const std::string&, const std::string&, const std::string&, bool);
	void AppendFName(const std::string& path, const std::string& name, std::string& outputpath)
	{
		outputpath = path;
		outputpath += '\\';
		outputpath += name;
	}
	char PathSeparator() const
	{ return '\\'; }
private:
	bool _ValidFname(const std::string&) const;
	bool DirCopy(const std::string&, const std::string&, bool) const;
	const std::string _host;
};


}

#else
	#error File already included
#endif
