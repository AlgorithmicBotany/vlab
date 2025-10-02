#ifndef __LINETHUMBCALLBACK_H__
#define __LINETHUMBCALLBACK_H__


class LineThumbCallback
{
public:
	virtual void Moved(float, bool) = 0;
};



#else
	#error File already included
#endif
