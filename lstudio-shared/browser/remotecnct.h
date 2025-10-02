#ifndef __REMOTECNCT_H__
#define __REMOTECNCT_H__

#include "socket.h"

namespace VLB
{

class RemoteConnection : public Connection
{
public:
	RemoteConnection(const char* host, const char* user, const char* pswd, const string_buffer& crlf);
	~RemoteConnection();
	bool HasExtensions(const std::string& path);
	bool GetExtensions(string_buffer&, string_buffer&, std::vector<int>&, const char*);
	bool GetOrdering(const char* parent, std::vector<std::string>& order);
	bool WriteOrdering(const char* parent, const std::vector<std::string>& order);
	bool GetParentPath(const std::string& path,std::string& parent);
	bool GetFileList(string_buffer&, const char*, bool);
	bool RenameObj(const std::string&, std::string&, const char*);
	bool PutObject(const string_buffer&);
	bool GetFile(const char*);
	bool GetFile(const char*, const char*);
	bool GetTmpFile(const char*, const char*, std::string&);
	const char* Host() const
	{ return _host.c_str(); }
	bool PutFile(const char*, const char*);
	bool PutFile(const char* /* remote path */, const char* /* local name */, const char* /* remote name */);
	bool MakeExt(const char*, const char*, std::string&);
	bool MakeHyperobject(const char* /*path*/, const char* /*name*/, const tGUID& uuid);
	bool DelTree(const char*);
	bool PrototypeObject(const char*);
	bool CompareFiles(const char*, const char*);
	bool DeleteFile(const char*, const char*);
	bool RequiresIds() const { return true; }
	bool SupportsCopyPaste() const { return true; }
	bool SupportsHyperCopy() const { return true; }
	bool Paste(const char*, const char*, const char*, bool);
	void AppendFName(const char* path, const char* name, std::string& outputpath)
	{
		outputpath = path;
		outputpath += '/';
		outputpath += name;
	}
	char PathSeparator() const
	{ return '/'; }
	bool SupportsTar() const
	{ return true; }
	bool Archive(TmpFile&, const char*, const char*, bool);
	bool Dearchive(TmpFile&, const char*, const char*, const char*);
	bool FindObject(const std::string&, const std::string&, const ObjectFindInfo&, std::string&);
	bool IsHyperobject(const std::string& path);
	bool LookupPath(const tGUID& uuid, const char* oofsroot,
			std::string& path);

	static bool ValidPath(const std::string& path);
	static bool ValidFname(const std::string& filename);

	void LoadUUIDBase(const std::string& path);
	void ReloadUUIDBase();

	bool ReconcileGuids(const std::string& oofs, const std::string& path);
	bool FixOofs();

private:
	bool GetUUID(const char* path, tGUID& uuid, std::string& name);
	bool GetUUID(const char* oofsroot, const char* path, tGUID& id);
	const char* _RelPath(const char* root, const char* objpath);
	bool _RetrieveMultipleStrings(string_buffer&, const char*);
	bool _RetrieveMultipleFiles(const char*);
	int _RetrieveInt(const unsigned char*);
	int _RetrieveIntRev(const unsigned char*);
	void _BuildIntRev(int, unsigned char*);
	bool _Login(const char*, const char*);
	bool _MakeDir(const char*, size_t);
	void _ValidateFName(std::string&) const;
	
	const std::string _host;
	SocketUser _su;
	Socket _socket;

	bool ConvertCRLF(const std::string&) const;
	string_buffer _crlf;
	static const int ArchiveBlockSize;


	std::string _lastUuidsOofs;
	UUidsBase _uuidsBase;

};

}


#else
	#error File already included
#endif
