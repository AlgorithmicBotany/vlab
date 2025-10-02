#ifndef __FINDFILE_H__
#define __FINDFILE_H__


class FindFile
{
public:
	FindFile(const std::string&);
	~FindFile();
	const std::string& FileName() const
	{ return _fname; }
	bool Found() const
	{ return (INVALID_HANDLE_VALUE != _hFind); }
	bool FindNext();
	bool IsDirectory() const
	{ return 0 != (_find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY); }
	bool StartsWith(const std::string&) const;
	bool EndsWith(const std::string&) const;
	bool IsSubDirectory() const;
	bool FilenameIs(const std::string&) const;
	static bool FileExists(const std::string& fnm);
private:
	HANDLE _hFind;
	WIN32_FIND_DATA _find;
	std::string _fname;
};


#endif
