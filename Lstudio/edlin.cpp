#include <fw.h>

#include "panelprms.h"
#include "edlin.h"

#include "resource.h"

EdLin::EdLin(const char* initstr)
{
	_size = eDefSize;
	_len = strlen(initstr);
	assert(_len<_size);
	strcpy(_bf, initstr);
}

EdLin::~EdLin()
{}

int EdLin::Find(int ix, char c) const
{
	assert(ix>=0);
	assert(ix<_len);
	while (_bf[ix] != 0)
	{
		if (c == _bf[ix])
			return ix;
		ix++;
	}
	return -1;
}


int EdLin::SkipBlanks(int ix) const
{
	assert(ix>=0);
	assert(ix<_len);
	while (_bf[ix] != 0)
	{
		if (!(isspace(_bf[ix])))
			return ix;
		ix++;
	}
	return -1;
}


void EdLin::Insert(int ix, const char* str)
{
	assert(ix>=0);
	assert(ix<=_len);

	int strl = strlen(str);
	if (_len+strl>=_size)
		_Grow(_len+strl);
	memmove(_bf+ix+strl, _bf+ix, _len-ix+1);
	memcpy(_bf+ix, str, strl);
	_len += strl;
}


void EdLin::DeleteChars(int ix, const char* list)
{
	assert(ix>=0);
	assert(ix<_len);

	while (_bf[ix] != 0)
	{
		if (strchr(list, _bf[ix]))
			DeleteChar(ix);
		else
			break;
	}
}


void EdLin::DeleteChar(int ix)
{
	assert(ix>=0);
	assert(ix<_len);
	memmove(_bf+ix, _bf+ix+1, _len-ix);
	_len--;
}



void EdLin::_Grow(int)
{
	throw Exception(IDERR_EDLINLINETOOLONG);
}
