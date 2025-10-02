#ifndef __DESCRIPTIONEDIT_H__
#define __DESCRIPTIONEDIT_H__

class DescriptionName
{
public:
	static const char* Name()
	{ return "description.txt"; }
};

typedef TextEdit<DescriptionName, IDD_DESCRIPTION> DescriptionEdit;


#else
	#error File already included
#endif
