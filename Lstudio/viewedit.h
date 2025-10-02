#ifndef __VIEWEDIT_H__
#define __VIEWEDIT_H__

class ViewName
{
public:
	static const char* Name()
	{ return "view.v"; }
};

typedef TextEdit<ViewName, IDD_VIEWDUMMY> ViewEdit;



#else
	#error File already included
#endif
