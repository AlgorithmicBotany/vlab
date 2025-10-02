#ifndef __PARAMS_H__
#define __PARAMS_H__

namespace Params
{
	const char LSspecs[] = "LSspecifications";
	const char specs[] = "specifications";
	const char DescriptionFName[] = "description.txt";

	namespace Snap
	{
		enum
		{
			nWidth = 139,
			nHeight = 139,
			nThickness = 2,
			nMinWidth = 8,
			nMinHeight = 8
		};
	}
}

#else
#error File already included
#endif
