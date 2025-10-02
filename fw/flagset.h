#ifndef __FLAGSET_H__
#define __FLAGSET_H__


class FlagSet
{
public:
	FlagSet() : _val(0L) {}
	bool IsSet(unsigned int v) const
	{ return (v == (v & _val)); }
	void Set(unsigned int v, bool b)
	{
		if (b)
			_val |= v;
		else
			_val &= ~v;
	}
private:
	unsigned int _val;
};


#endif
