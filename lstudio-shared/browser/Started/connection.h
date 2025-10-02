#ifndef __CONNECTION_H__
#define __CONNECTION_H__


namespace VLB
{

class Connection
{
public:
	virtual ~Connection() {}
	virtual bool HasExtensions(const std::string&) = 0;
	virtual bool GetExtensions(string_buffer&, string_buffer&, std::vector<int>&, const std::string&) = 0;
	virtual bool GetFileList(string_buffer&, const std::string&, bool) = 0;
	virtual bool RenameObj(const std::string&, std::string&, const std::string&) = 0;
	bool PutObject(const string_buffer& flist, const std::string& lbtbl, const std::string& path)
	{
		for (string_buffer::const_iterator it(flist); !it.at_end(); it.advance())
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
	virtual bool FetchObject(const std::string&) = 0;
	virtual bool GetFile(const std::string&) = 0;
	virtual bool GetFile(const std::string&, const std::string&) = 0;
	virtual bool GetTmpFile(const std::string& /* rempath */, const std::string& /* fname */, std::string& /* local path+name */) = 0;
	virtual bool AddId(const std::string& /*path*/, const std::string& /*oofsroot*/) = 0;
	virtual bool DeleteIds(const std::string& /* path */, const std::string& /* oofs root */) = 0;
	virtual const std::string& Host() const = 0;
	virtual bool PutFile(const std::string& /* src full path */, const std::string& /* trg full path */) = 0;
	virtual bool PutFile(const std::string& /* remote path */, const std::string& /* local name */, const std::string& /* remote name */) = 0;
	virtual bool MakeExt(const std::string&, const std::string&, std::string&) = 0;
	virtual bool DelTree(const std::string&) = 0;
	virtual bool PrototypeObject(const std::string&) = 0;
	virtual bool CompareFiles(const std::string&, const std::string&) = 0;
	virtual bool RequiresIds() const = 0;
	virtual bool SupportsCopyPaste() const = 0;
	virtual bool DeleteFile(const std::string&, const std::string&) = 0;
	virtual bool Paste(const std::string& oofs, const std::string& src, const std::string& trg, bool) = 0;
	virtual void AppendFName(const std::string& path, const std::string& name, std::string& outputpath) = 0;
	virtual char PathSeparator() const = 0;
};


}


#else
	#error File already included
#endif
