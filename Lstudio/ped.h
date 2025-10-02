#ifndef __PED_H__
#define __PED_H__



class Ped
{
public:
	virtual ~Ped() {}
	void Action(const char*);
private:
	void _DoD(const char*);
	void _DoN(const char*);
	void _DoO(const char*);
	void _DoNum(const char*);

	virtual void _OpenSrc() = 0;
	virtual void _ReadLine(char*, int) = 0;
	virtual void _CopyRest(TmpFile&) = 0;
	virtual void _Commit(TmpFile&) = 0;

	class BufFile : public TempPath
	{
	public:
		BufFile()
		{ append(FileName()); }
		static const char* FileName()
		{ return "pedbf"; }
	};

};

class FilePed : public Ped
{
public:
	FilePed(const char*);
	~FilePed();
private:
	void _OpenSrc();
	void _ReadLine(char*, int);
	void _CopyRest(TmpFile&);
	void _Commit(TmpFile&);

	char _Fname[_MAX_PATH+1];
	FILE* _fp;
	int _curline;
};


class WndPed : public Ped
{
public:
	WndPed(HWND);
	~WndPed();
	void _OpenSrc();
	void _ReadLine(char*, int);
	void _CopyRest(TmpFile&);
	void _Commit(TmpFile&);
private:
	int _curline;
	EditLine _target;
	const int _maxlines;
};

#else
	#error File already included
#endif
