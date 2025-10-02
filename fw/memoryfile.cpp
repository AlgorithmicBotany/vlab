
#include <string>
#include <cassert>

#include <windows.h>

#include "warningset.h"

#include "memoryfile.h"
#include "exception.h"

#include "libstrng.h"


MemoryMappedFile::MemoryMappedFile(const std::string& fname, DWORD access, DWORD share, DWORD disp)
{
	_hFile = CreateFile
		(
		fname.c_str(), 
		access, share, 
		0, 
		disp, 0, 0
		);

	if (INVALID_HANDLE_VALUE == _hFile)
		throw Exception(0, FWStr::OpenFile, fname.c_str());


}



MemoryMappedFile::~MemoryMappedFile()
{

	CloseHandle(_hFile);
}

