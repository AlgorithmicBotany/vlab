#include <fw.h>

#include <GL/gl.h>

#include "displists.h"
#include "liboglstrng.h"


DisplayLists::DisplayLists()
{
	_base = 0;
	_range = 0;
}

void DisplayLists::Init(int range) 
{
	assert(_base == 0);
	_range = range;
	assert(_range>0);
	_base = glGenLists(_range);
	if (0==_base)
		throw Exception(GetLibOglString(FWOGLStr::GenerateList));
}

DisplayLists::~DisplayLists()
{
	if (_base != 0)
		glDeleteLists(_base, _range);
}
