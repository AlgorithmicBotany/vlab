#include <windows.h>

#include "warningset.h"

#include "scrnpnt.h"



bool operator==(POINT a, POINT b)
{
	if (a.x != b.x)
		return false;
	if (a.y != b.y)
		return false;
	return true;
}
