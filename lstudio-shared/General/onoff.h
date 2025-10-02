#ifndef __ONOFF_H__
#define __ONOFF_H__

class OnOff
{
public:
	OnOff(bool& flag) : _flag(flag)
	{ _flag = true; }
	~OnOff()
	{ _flag = false; }
private:
	bool& _flag;
};


#else
	#error File already included
#endif
