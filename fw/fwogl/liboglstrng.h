#ifndef __LIBOGLSTRINGS_H__
#define __LIBOGLSTRINGS_H__



const char* GetLibOglString(int);

namespace FWOGLStr
{
	enum
	{
		strZero = 0,
		ReadingMaterial,
		InitOpenGL,
		GenerateList,

		nmLastString
	};
}

#endif
