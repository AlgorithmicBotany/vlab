#include <cassert>

#include <warningset.h>

#include "liboglstrng.h"



const char* FWOGLstrings[] =
{
	"",
	"Error reading material file: %s",
	"Error initializing OpenGL window",
	"Cannot allocate display list"
};


const char* GetLibOglString(int ix)
{ 
	assert(ix>0);
	assert(ix<FWOGLStr::nmLastString);
	return FWOGLstrings[ix];
}
