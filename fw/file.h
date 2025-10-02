/**************************************************************************

  File:		file.h
  Created:	11-Dec-97


  Declaration of file classes


**************************************************************************/


#ifndef __FILE_H__
#define __FILE_H__



class File
{
public:
	~File();
	const char* Filename() const
	{ return _fname.c_str(); }
	FILE* Handle()
	{ return _fp; }
protected:
	File(const std::string&, const std::string&);

	FILE* _Fp() const
	{ return _fp; }
private:

	FILE* _fp;

	std::string _fname;
};

class ReadBinFile : public File
{
public:
	ReadBinFile(const std::string& fname) : File(fname, __TEXT("rb"))
	{}
	void Read(void*, size_t, size_t count = 1);
	long Size();
};

class WriteBinFile : public File
{
public:
	WriteBinFile(const std::string& fname) : File(fname, __TEXT("wb"))
	{}
	void Write(const void*, size_t, size_t count = 1);
};


class ReadTextFile : public File
{
public:
	ReadTextFile(const std::string& fname) : File(fname, __TEXT("rt"))
	{ 
		_Eof = false; 
		_line = 0;
	}
	//void Read(char*, int maxlen = 64);
	void Read(std::string&);
	char GetChar();
	bool Eof()
	{ return _Eof; }
	int Line() const
	{ return _line; }

private:
	bool _Eof;
	int _line;
};

class WriteTextFile : public File
{
public:
	WriteTextFile(const std::string& fname) : File(fname, __TEXT("wt")){}

	void PrintF(const char*, ...);
	void Write(const char*);
	void Write(const char*, int);
	void Write(char);
	void WriteLn(const char*);
	void WriteEOL();
};



#endif
