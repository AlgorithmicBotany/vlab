#include <warningset.h>

#include <fw.h>

#include "ptslookup.h"

void PtsLookup::Delete(int n)
{
	assert(n>=0);
	assert(n<_items);
	for (int i=n; i<_items-1; i++)
		_arr[i] = _arr[i+1];
	_items--;
}
