#ifndef __RESSTRING_H__
#define __RESSTRING_H__


class ResString : public std::string
{
public:
	ResString(size_t sz, UINT id);
	void Load(UINT id);
private:
	size_t _initSize;
};



#endif
