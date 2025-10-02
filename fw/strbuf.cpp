#include <algorithm>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cctype>

#include "warningset.h"

#include "strbuf.h"

enum prms
{ eDefSize = 64 };

const size_t string_buffer::npos = static_cast<size_t>(-1);

string_buffer::string_buffer()
{
	_bf = reinterpret_cast<char*>(malloc(eDefSize));
	_endpos = 0;
	_size = eDefSize;
}


string_buffer::string_buffer(const string_buffer& src)
{
	_bf = reinterpret_cast<char*>(malloc(src._size));
	_size = src._size;
	memcpy(_bf, src._bf, _size);
	_endpos = src._endpos;
}


string_buffer::~string_buffer()
{
	free(_bf);
}

void string_buffer::add(char ch)
{
	if (_endpos==_size)
		_Grow();
	_bf[_endpos] = ch;
	_endpos++;
}

void string_buffer::add(const char* str)
{
	size_t l = strlen(str);
	assert(l>0);
	while (_endpos+l+1>_size)
		_Grow();
	strncpy(_bf+_endpos, str, l);
	_bf[_endpos+l] = 0;
	_endpos += l+1;
}

const char* string_buffer::add_str(const char* str)
{
	while (!isspace(str[0]) && 0 != str[0])
	{
		add(str[0]);
		++str;
	}
	add('\0');
	return str;
}

void string_buffer::_Grow()
{
	_bf = reinterpret_cast<char*>(realloc(_bf, _size*2));
	_size *= 2;
}


size_t string_buffer::find_next(size_t pos) const
{
	if (pos>=_endpos)
		return npos;
	while (0 != _bf[++pos])
		continue;
	if (pos+1==_endpos)
		return npos;
	else
		return ++pos;
}


size_t string_buffer::find_prev(size_t pos) const
{
	assert(pos != npos);
	if (0==pos)
		return npos;
	--pos;
	assert(0==_bf[pos]);
	if (0==pos)
		return pos;
	--pos;
	while (0 != _bf[pos] && pos>0)
		--pos;
	if (0==pos)
		return pos;
	else
		return pos+1;
}


size_t string_buffer::last_string() const
{
	if (is_empty())
		return npos;
	size_t res = _endpos-2;
	while (0 != _bf[res] && res>0)
		--res;
	if (0==res)
		return res;
	else
		return res+1;
}




void string_buffer::erase(const string_buffer::iterator& it)
{
	erase(it.pos());
}

void string_buffer::erase(size_t pos)
{
	assert(pos<_endpos);
	size_t l = strlen(_bf+pos);
	memmove(_bf+pos, _bf+pos+l+1, _endpos-pos-l);
	_endpos -= l+1;
}


bool string_buffer::contains(const char* txt) const
{
	return find(txt) != npos;
}

size_t string_buffer::find(const char* txt) const
{
	size_t pos = begin();
	while (npos != pos)
	{
		if (0==strcmp(txt, string(pos)))
			return pos;
		pos = find_next(pos);
	}
	return npos;
}

bool string_buffer::const_iterator::is(const char* strng) const
{ return 0 == strcmp(strng, str());}

bool string_buffer::iterator::is(const char* strng) const
{ return 0 == strcmp(strng, str());}

bool string_buffer::const_iterator::starts_with(const char* strng) const
{
	size_t l = strlen(str());
	size_t cl = strlen(strng);
	if (cl>l)
		return false;
	return 0==strncmp(str(), strng, cl);
}

bool string_buffer::iterator::starts_with(const char* strng) const
{
	size_t l = strlen(str());
	size_t cl = strlen(strng);
	if (cl>l)
		return false;
	return 0==strncmp(str(), strng, cl);
}



void string_buffer::sort()
{
	std::vector<Entry> entries;
	for (const_iterator it(*this); !it.at_end(); it.advance())
	{
		Entry e = { it.pos(), const_cast<char*>(it.str()) };
		entries.push_back(e);
	}
	std::sort(entries.begin(), entries.end(), Entry::Compare);
	string_buffer sorted;
	for (std::vector<Entry>::const_iterator it = entries.begin(); it != entries.end(); ++it)
		sorted.add(it->txt);
	swap(sorted);
}
