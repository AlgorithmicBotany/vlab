#ifndef __COMMONDEFS_H__
#define __COMMONDEFS_H__


namespace PrjVar
{
	bool IsEvalVer();

#include "dt.h"

	inline bool IsExperimental()
	{ return false; }

	const bool CpfgViewer = false;

	enum CpfgCommand
	{
		ccNewModel = 44001, // New model
		ccNewLsystem,
		ccNewHomomorphism,
		ccNewView,
		ccRerun,
		ccNewProjection,
		ccNewRender,
		ccNewAnimate,
		ccRereadColors,
		ccRereadSurfaces,
		ccRereadCurvesRerun,
		ccRereadContours,
		ccRereadFunctionsRerun,

		ccQuitting    = 40055,
		ccPanicExit   = 40077
	};
}


#else
	#error File already included
#endif
