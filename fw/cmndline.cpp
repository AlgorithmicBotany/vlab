#include <cassert>
#include <cstdlib>

#include "warningset.h"

#include "cmndline.h"

int CommandLine::ArgC() const
{ 
	return __argc;
}


const char* CommandLine::ArgV(int i) const
{
	assert(i>=0);
	assert(i<ArgC());
	return __argv[i];
}
