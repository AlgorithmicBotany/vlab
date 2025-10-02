#include <fw.h>

#include "animdata.h"

AnimData::AnimData()
{
	Default();
}


void AnimData::Default()
{
	_flags.Set(ftDoubleBuffer, true);
	_flags.Set(ftClearBF, true);
	_flags.Set(ftScaleBF, false);
	_flags.Set(ftNewViewBF, false);
	_flags.Set(ftHCenterBF, false);
	_flags.Set(ftDisplayOnRequest, false);
	_FrameNumbers = nfConsecutive;
	_SwapInterval = 1;
	_FirstFrame = 0;
	_LastFrame = -1;
	_Step = 1;
	_Intervals[0] = 0;
}
