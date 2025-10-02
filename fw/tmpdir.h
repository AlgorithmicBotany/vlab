#ifndef __TMPDIR_H__
#define __TMPDIR_H__



class TempPath : public std::string
{
public:
	TempPath()
	{
		const size_t tmp_size = 2;
		char tmp[tmp_size];
		size_t l = ::GetTempPath(tmp_size, tmp);
		resize(l+1);
		l = ::GetTempPath(l+1, &(at(0)));
		resize(l);
	}
};


class CurrentDirectory : public std::string
{
public:
	CurrentDirectory()
	{
		GetCurrentDirectory(*this);
	}
	CurrentDirectory(std::string& output)
	{
		GetCurrentDirectory(*this);
		output = *this;
	}
private:
	static void GetCurrentDirectory(std::string& output)
	{
		const size_t tmp_size = 2;
		char tmp[tmp_size];
		size_t l = ::GetCurrentDirectory(tmp_size, tmp);
		output.resize(l+1);
		l = ::GetCurrentDirectory(l+1, &(output.at(0)));
		output.resize(l);
	}
};

class TmpChangeDir
{
public:
	TmpChangeDir();
	TmpChangeDir(const std::string& dir);
	~TmpChangeDir()
	{ ::SetCurrentDirectory(_Original.c_str()); }
	const std::string& OrigDir() const
	{ return _Original; }
private:
	std::string _Original;
};




#endif
