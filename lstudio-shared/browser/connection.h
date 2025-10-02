#ifndef __CONNECTION_H__
#define __CONNECTION_H__


#include <fstream>

#include "tGuid.h"


namespace VLB
{

class ObjectFindInfo
{
public:
	ObjectFindInfo()
	{
		name = "";
		matchCase = false;
		wholeName = false;
	}
	std::string& Name() 
	{ return name; }
	bool& MatchCase() 
	{ return matchCase; }
	bool& WholeName() 
	{ return wholeName; }
	const std::string& Name() const
	{ return name; }
	bool MatchCase() const
	{ return matchCase; }
	bool WholeName() const
	{ return wholeName; }
private:
	std::string name;
	bool matchCase;
	bool wholeName;
};


class Connection
{
public:

	enum eRAserverType
	{
		tRALink = 1,
		tRAHasExtensions = 2
	};


	virtual ~Connection() {}

	// Does the object at the given path have extensions?
	virtual bool HasExtensions(const std::string& path) = 0;

	// Get all extensions of the object at the given path;
	// returns names, fullnames, and types of the extensions
	virtual bool GetExtensions(string_buffer& names,
				   string_buffer& fullnames,
				   std::vector<int>& types,
				   const char* path) = 0;

	// Returns the oofs-specified ordering of the path's
	// children. This may not contain all children, and may even
	// contain names which are not children.
	virtual bool GetOrdering(const char* parent,
				 std::vector<std::string>& order) = 0;

	// Writes the given order as the oofs-specified ordering of
	// the path's children.
	virtual bool WriteOrdering(const char* parent,
				   const std::vector<std::string>& order) = 0;

	// Gets the path of the parent of the given path.
	virtual bool GetParentPath(const std::string& path,
				   std::string& parent) = 0;

	// Gets a list of all files in the given path. The flag filesonly
	// (list only files, not directories) is only applied in the case of
	// local oofs.
	virtual bool GetFileList(string_buffer& list, const char* path,
				 bool filesonly) = 0;

	// Renames the object at path in oofs to the newname
	virtual bool RenameObj(const std::string& oofs, std::string& path,
				const char* newname) = 0;

	// Copies files listed in flist from lbtbl to path.
	bool PutObject(const string_buffer& flist,
		const std::string& lbtbl, const std::string& path)
	{
		for (string_buffer::const_iterator it(flist);
			!it.at_end(); it.advance())
		{
			std::string src(lbtbl);
			src.append(1, PathSeparator());
			src.append(it.str());
			std::string trg(path);
			trg.append(1, PathSeparator());
			trg.append(it.str());
			if (!PutFile(src.c_str(), trg.c_str()))
				return false;
		}
		return true;
	}

	// Fetches the file at fname to the current working directory
	virtual bool GetFile(const char* fname) = 0;

	// Fetches the file at fname to the target file
	virtual bool GetFile(const char* fname, const char* target) = 0;

	// Fetches the file at rempath/fname into a temporary file, whose
	// name is returned in locfile
	virtual bool GetTmpFile(const char* rempath, const char* fname,
				std::string& locfile) = 0;

	// Returns the oofs host ("[local]" for local oofs)
	virtual const char* Host() const = 0;

	// Puts the file from src_local into trg_remote
	virtual bool PutFile(const char* src_local,
			     const char* trg_remote) = 0;

	// Puts file from local_name into remote_path/remote_name
	virtual bool PutFile(const char* remote_path,
			     const char* local_name,
			     const char* remote_name) = 0;

	// Makes an extension name to the object in location, returning
	// new object path in dirname.
	virtual bool MakeExt(const char* location, const char* name,
			     std::string& dirname) = 0;

	// Makes the object in location a hyperobject referencing
	// the object with id.
	virtual bool MakeHyperobject(const char* location, const char* name, const tGUID& id) = 0;

	// loads the uuid database if applicable
	virtual void LoadUUIDBase(const std::string& path) = 0;

	// reloads uuids database from the last specified location
	virtual void ReloadUUIDBase() = 0;

	// Deletes the tree rooted at path
	virtual bool DelTree(const char* path) = 0;

	// Compares an object's files to the parent and symlinks
	// as appropriate.
	virtual bool PrototypeObject(const char*) = 0;

	// Compares local file fname with remote file remote_path/fname
	virtual bool CompareFiles(const char* fname,
				  const char* remote_path) = 0;

	// Does this connection support copy/paste operations?
	virtual bool SupportsCopyPaste() const = 0;

	// Does this connection support hyper objects?
	virtual bool SupportsHyperCopy() const = 0;

	// Deletes the remote file path/fname
	virtual bool DeleteFile(const char* path, const char* fname) = 0;

	// Pastes object (or whole tree) from src to trg (both remote).
	virtual bool Paste(const char* oofs, const char* src,
			   const char* trg, bool recursive) = 0;

	// Creates the filename path/name (or equivalent) and returns
	// in outputpath
	virtual void AppendFName(const char* path, const char* name,
				 std::string& outputpath) = 0;

	// The path separator on this connection
	// (e.g. / for raserver, \ for windows)
	virtual char PathSeparator() const = 0;

	// Does this connection support tarfiles? (raserver does)
	virtual bool SupportsTar() const = 0;

	// Archive tree at path to tmpfile, perhaps recursively.
	virtual bool Archive(TmpFile& tmpfile,
			     const char* oofsroot, const char* path,
			     bool recursive) = 0;

	// Dearchives from tmpfile into topath, via paste request
	// from srcpath.
	virtual bool Dearchive(TmpFile& tmpfile,
			       const char* oofsroot, const char* srcpath,
			       const char* topath) = 0;

	// Finds a file described by fi in oofs, starting from object
	// start_from, returning first match in output.
	virtual bool FindObject(
		const std::string& oofs,
		const std::string& start_from, 
		const ObjectFindInfo& fi, std::string& output) = 0;

	// Is the object at path a hyperobject?
	virtual bool IsHyperobject(const std::string& path) = 0;

	// Looks up the given id in the .dbase of oofsroot
	// and returns the object path in path.
	virtual bool LookupPath(const tGUID& id, const char* oofsroot,
				std::string& path) = 0;

	// Looks up the id number of path (from the .id file if an object,
	// or from the node file if a hyperobject). Returns in id.
	virtual bool GetUUID(const char* path, tGUID& id, std::string& name) = 0;

	// sends GET_UUID message to RA server
	// if a new guid is generated
	// this method will update connection's guid database as necessary
	virtual bool GetUUID(const char* oofsroot, const char* path, tGUID& id) = 0;

	// Returns the name of an object (the last component of the
	// path if an object; if a hyperobject, from the node file or,
	// if blank, from the referenced object).
	virtual bool GetObjectName(const std::string& path,
		const std::string& /*oofsroot*/,
		std::string& name)
	{
		// The default implementation just returns the last
		// component of the path.
		size_t pos = path.find_last_of(PathSeparator());
		if(pos != std::string::npos)
			name = path.substr(pos+1);
		else
			name = path;
		return true;
	}

	virtual bool GetHyperobjectInfo(const std::string& oofsRoot, const std::string& path, tGUID& guid, std::string& targetPath, std::string& name)
	{
		bool bRes = GetUUID(path.c_str(), guid, name);
		if (!bRes)
			return false;
		bRes = LookupPath(guid, oofsRoot.c_str(), targetPath);
		if (bRes && targetPath[targetPath.size()-1] == PathSeparator())
		{
			targetPath.erase(targetPath.length()-1);
		}
		return bRes;
	}

	virtual bool ReconcileGuids(const std::string& oofs, const std::string& path) = 0;
	virtual bool FixOofs() = 0;


protected:



	class UUidsBase
	{
	public:
		UUidsBase() {}
	
		void Load(const std::string& fname)
		{
			_Entries.clear();
			::OutputDebugStringA("UUidsBase constructed\n");
			std::ifstream source(fname);
			if (!source.is_open())
				return;
			VLB::tGUID uid;
			std::string path;
			std::string uidString;
			source >> uidString >> path;
			uid.FromString(uidString);
			while (source.good())
			{
				Entry entry(uid, path);
				_Entries.push_back(entry);
				source >> uidString >> path;
				uid.FromString(uidString);
			}
		}
		void Save(const std::string& fname) const
		{
			std::ofstream target(fname);
			std::string uidString;
			for (CIterator it = _Entries.begin(); it != _Entries.end(); ++it)
			{
				it->_uuid.ToString(uidString);
				target << uidString << ' ' << it->_path << std::endl;
			}
		}

		bool LookupPath(const VLB::tGUID& uuid, std::string& outPath) const
		{
			for (CIterator it = _Entries.begin(); it != _Entries.end(); ++it)
			{
				if (it->_uuid == uuid)
				{
					outPath = it->_path;
					return true;
				}
			}
			return false;
		}

		bool LookupGuid(const std::string& path, VLB::tGUID& result) const
		{
			for (CIterator it = _Entries.begin(); it != _Entries.end(); ++it)
			{
				if (it->_path == path)
				{
					result = it->_uuid;
					return true;
				}
			}
			return false;
		}

		void AddEntry(const VLB::tGUID& uuid, const std::string& path)
		{
			Entry entry(uuid, path);
			_Entries.push_back(entry);
		}
	private:
		struct Entry
		{
			Entry(const VLB::tGUID& uuid, const std::string& path) : _uuid(uuid), _path(path)
			{}
			VLB::tGUID _uuid;
			std::string _path;
		};
		std::vector<Entry> _Entries;
		typedef std::vector<Entry>::const_iterator CIterator;
	};

};


}


#else
	#error File already included
#endif
