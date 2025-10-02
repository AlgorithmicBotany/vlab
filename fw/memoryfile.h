#ifndef __MEMORYFILE_H__
#define __MEMORYFILE_H__


class MemoryMappedFile
{
public:
	MemoryMappedFile(const std::string&, DWORD access, DWORD share, DWORD dist);
	~MemoryMappedFile();
	void Read(void*, DWORD);
	void Write(const void*, DWORD);
	void Seek(LONG);
private:
	HANDLE _hFile;
	HANDLE _hMap;
};

#endif
