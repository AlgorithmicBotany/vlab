#ifndef __TMPFILE_H__
#define __TMPFILE_H__



class TempFileName : public std::string
{
public:
	TempFileName(const std::string& prefix);
};

class TmpFile
{
public:
	TmpFile(const TCHAR*, DWORD, DWORD, DWORD);
	TmpFile(bool bOpen = true);
	~TmpFile();
	void Write(const void*, DWORD);
	void Read(void*, DWORD);
	void Reset();
	bool Eof() const
	{ return _eof; }
	// resets the pointer
	int Size() const;
	bool IsOpen() const
	{ return _hFile != INVALID_HANDLE_VALUE; }
	const std::string& Filename() const 
	{ return _fname; }
private:
	std::string _fname;
	HANDLE _hFile;
	bool _eof;
};


#endif
