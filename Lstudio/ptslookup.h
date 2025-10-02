#ifndef __POINTSLOOKUP_H__
#define __POINTSLOOKUP_H__


class PtsLookup : public DynArray<int>
{
public:
	void Delete(int i);
	void operator=(const PtsLookup&);
};

#else
	#error File already included
#endif
