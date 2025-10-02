#ifndef __DISPLAYLISTS_H__
#define __DISPLAYLISTS_H__

class DisplayLists
{
public:
	DisplayLists();
	~DisplayLists();
	void Init(int);
	unsigned int Base() const
	{ return _base; }
private:
	unsigned int _base;
	int _range;
};

#endif
