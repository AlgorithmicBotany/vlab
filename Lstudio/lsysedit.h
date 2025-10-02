#ifndef __LSYSTEMEDIT_H__
#define __LSYSTEMEDIT_H__

class LSystemName
{
public:
	static const char* Name()
	{ return "lsystem.l"; }
};

typedef TextEdit<LSystemName, IDD_LSYSTEM> LSystemEdit;


#else
	#error File already included
#endif
