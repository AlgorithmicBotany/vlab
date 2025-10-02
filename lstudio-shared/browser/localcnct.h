#ifndef __LOCALCNCT_H__
#define __LOCALCNCT_H__

namespace VLB
{

class LocalConnection : public Connection
{
public:
	LocalConnection();
	~LocalConnection();
	bool HasExtensions(const std::string& path);
	bool GetExtensions(string_buffer&, string_buffer&, std::vector<int>&, const char*);
	bool GetOrdering(const char* szParentPath, std::vector<std::string>& order);
	bool WriteOrdering(const char* szParentPath, const std::vector<std::string>& order);
	bool GetParentPath(const std::string& path, std::string& parent);
	bool GetFileList(string_buffer&, const char*, bool);
	bool RenameObj(const std::string&, std::string&, const char*);
	bool PutObject(const string_buffer&);
	bool GetFile(const char*);
	bool GetFile(const char*, const char*);
	bool GetTmpFile(const char*, const char*, std::string&);
	const char* Host() const
	{ return "[local]"; }
	bool PutFile(const char*, const char*);
	bool PutFile(const char* /* remote path */, const char* /* local name */, const char* /* remote name */);
	bool MakeExt(const char*, const char*, std::string&);
	bool MakeHyperobject(const char* /*path*/, const char* /*name*/, const tGUID& /*uuid*/);
	void LoadUUIDBase(const std::string& /*path*/);
	void ReloadUUIDBase();
	bool DelTree(const char*);
	bool PrototypeObject(const char*);
	bool CompareFiles(const char*, const char*);
	bool DeleteFile(const char*, const char*);
	bool RequiresIds() const { return false; }
	bool SupportsCopyPaste() const { return true; }
	bool SupportsHyperCopy() const { return true; }
	bool Paste(const char*, const char*, const char*, bool);
	void AppendFName(const char* path, const char* name, std::string& outputpath)
	{
		outputpath = path;
		outputpath += '\\';
		outputpath += name;
	}
	char PathSeparator() const
	{ return '\\'; }
	bool SupportsTar() const
	{ return false; }
	bool Archive(TmpFile&, const char*, const char*, bool)
	{ return false; }
	bool Dearchive(TmpFile&, const char*, const char*, const char*)
	{ return false; }
	bool FindObject(const std::string&, const std::string&, const ObjectFindInfo&, std::string&) { return false;}
	bool IsHyperobject(const std::string&);
	bool LookupPath(const tGUID& /*uuid*/, const char*,std::string&);
	bool GetUUID(const char*, tGUID& /*uuid*/, std::string& /*name*/);
	bool GetUUID(const char* /*oofsroot*/, const char* /*path*/, tGUID& /*id*/);
	static bool ValidFname(const std::string&);

	bool ReconcileGuids(const std::string& /*oofs*/, const std::string& /*path*/);
	bool FixOofs();

private:

	void BuildOrderingFilename(const char* szParentPath, std::string& result) const;
	void BuildUUIDBaseFilename(const std::string& oofsroot, std::string& result) const;
	void BuildNodeFilename(const std::string& path, std::string& result) const;
	void BuildUUIDFilename(const std::string& path, std::string& result) const;
	
	bool DirCopy(const char*, const char*, bool) const;


	std::string _lastUuidsOofs;
	UUidsBase _uuidsBase;

};


}

#else
	#error File already included
#endif
