#include <cassert>
#include <cstdio>
#include <cstdlib>

#include <string>

#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "lstring.h"
#include "exception.h"
#include "file.h"
#include "rawmemory.h"



void RawMemory::Read(ReadBinFile& src, int offset, int size)
{
	assert(offset>=0);
	if (0==size)
		size = _size;
	assert(offset+size<=_size);
	src.Read(_bf + offset, size);
}

