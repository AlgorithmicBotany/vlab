#ifndef __RAWMEMORY_H__
#define __RAWMEMORY_H__


int OutOfMemory(size_t);

class ReadBinFile;

class RawMemory
{
public:
	RawMemory(int size) : _size(size)
	{
		_bf = (char*) malloc(_size);
		if (0 == _bf)
			OutOfMemory(_size);
	}
	~RawMemory()
	{
		free(_bf);
	}
	//template <typename T>
	char* Buffer(int offset = 0)
	{ 
		assert(offset>=0);
		assert(offset<_size);
		return _bf + offset; 
	}
	void Copy(int offset, const void* src, int bytes)
	{
		assert(offset<_size);
		assert(offset>=0);
		assert(bytes>0);
		assert(offset+bytes<=_size);
		memcpy(_bf+offset, src, bytes);
	}
	int Size() const
	{ return _size; }

	// if offset and size are zero fill all the buffer with file contents
	void Read(ReadBinFile&, int offset = 0, int size = 0);  
	char operator[](int i) const
	{ 
		assert(i>=0);
		assert(i<_size);
		return _bf[i];
	}
private:
	const int _size;
	char* _bf;
};


#endif
