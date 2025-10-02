#ifndef __STRBUF_H__
#define __STRBUF_H__

#include <vector>

class string_buffer
{
public:
	string_buffer();
	string_buffer(const string_buffer&);
	~string_buffer();

	void add(char);

	// adds the string up to (not including 0)
	void add(const char*);

	// adds the string up to (not including) first blank or 0
	// returns the first blank or 0 character after the added string
	const char* add_str(const char*);
	size_t begin() const
	{ 
		if (0==_endpos)
			return npos;
		else
			return 0; 
	}
	size_t last_string() const;
	void erase(size_t pos);
	const char* string(size_t pos) const
	{ 
		assert(pos<_endpos);
		return _bf+pos; 
	}
	static const size_t npos;
	size_t find_next(size_t) const;
	size_t find_prev(size_t) const;
	char at(size_t pos) const
	{
		assert(pos<_endpos);
		return _bf[pos];
	}
	bool in_range(size_t pos) const
	{ return pos<_endpos; }
	void reset()
	{ _endpos = 0; }
	bool contains(const char*) const;
	size_t find(const char*) const;
	bool is_empty() const
	{ return (0==_endpos); }
	class const_iterator
	{
	public:
		const_iterator(const string_buffer& sb) : _sb(sb), _pos(0) {}
		size_t pos() const
		{ return _pos; }
		bool at_end() const
		{ return !_sb.in_range(_pos); }
		const char* str() const
		{ 
			assert(!at_end());
			return _sb.string(_pos); 
		}
		void advance()
		{
			assert(!at_end());
			while (_sb.at(_pos) != 0)
				++_pos;
			++_pos;
		}
		bool is(const char*) const;
		bool starts_with(const char*) const;
	private:
		const string_buffer& _sb;
		size_t _pos;
	};
	class iterator
	{
	public:
		iterator(string_buffer& sb) : _sb(sb), _pos(0) {}
		size_t pos() const
		{ return _pos; }
		bool at_end() const
		{ return !_sb.in_range(_pos); }
		const char* str() const
		{ 
			assert(!at_end());
			return _sb.string(_pos); 
		}
		void advance()
		{
			assert(!at_end());
			while (_sb.at(_pos) != 0)
				++_pos;
			++_pos;
		}
		void erase_string()
		{ _sb.erase(*this); }
		bool is(const char*) const;
		bool starts_with(const char*) const;
	private:
		string_buffer& _sb;
		size_t _pos;
	};

	void erase(const iterator&);

	void sort();

	void swap(string_buffer& sb)
	{
		char* tmpbf = _bf;
		_bf = sb._bf;
		sb._bf = tmpbf;
		size_t tmps = _endpos;
		_endpos = sb._endpos;
		sb._endpos = tmps;
		tmps = _size;
		_size = sb._size;
		sb._size = tmps;
	}
private:
	void _Grow();
	char* _bf;
	size_t _endpos;
	size_t _size;
	void operator=(const string_buffer&);
	struct Entry
	{
		size_t pos;
		char* txt;
		static bool Compare(const Entry& E1, const Entry& E2)
		{ return strcmp(E1.txt, E2.txt)<0; }
	};
};

#endif
