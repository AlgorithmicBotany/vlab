
#ifndef __SCRAMBLE_H__
#define __SCRAMBLE_H__


class FileScrambler
{
public:
	FileScrambler(const char*);
	~FileScrambler();
	void Add(const char* fnm, const char* cntnts);
	void AddDir(const char* path);
	void Add(const char* fname);
private:
	void _Add(const char*, size_t);
	void _Add(char);
	std::ofstream _output;
	typedef unsigned short int ChkSumType;
	ChkSumType _chksm;
	class Buffer
	{
	public:
		Buffer() : _last(0) {}
		bool Full() const
		{ return BufferSize==_last; }
		bool Empty() const
		{ return 0==_last; }
		void Add(char c)
		{
			assert(!Full());
			_bf[_last] = c;
			++_last;
		}
		void Shuffle()
		{
			for (int i=0; i<BufferSize; ++i)
				_bf[i] = (char) ~_bf[i];
		}
		void Flush(std::ofstream& trg)
		{
			trg.write(_bf, BufferSize);
			_last = 0;
		}
	private:
		enum
		{ BufferSize = 4 };
		int _last;
		char _bf[BufferSize];
	};
	Buffer _buffer;
};



class FileDescrambler
{
public:
	FileDescrambler(const char* fname) : _buffer(fname)
	{}
	void Extract();
private:
	void _ExtractFName(std::string&);
	void _ExtractFile(const char*);

	class Buffer
	{
	public:
		Buffer(const char* fname); 
		void Get(char* bf, size_t sz)
		{
			for (size_t n=0; n<sz; ++n)
				bf[n] = _Get();
		}
		char GetChar()
		{ return _Get(); }

	private:
		enum
		{ BufferSize = 4 };
		char _Get();
		void _Deshuffle()
		{
			for (int i=0; i<BufferSize; ++i)
				_bf[i] = (char) ~_bf[i];
		}

		std::ifstream _src;

		char _bf[BufferSize];
		int _last;
	};
	Buffer _buffer;
	typedef unsigned short int ChkSumType;
};




#else
	#error File already included
#endif
