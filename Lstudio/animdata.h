#ifndef __ANIMDATA_H__
#define __ANIMDATA_H__


class AnimData
{
public:
	AnimData();
	bool DoubleBuffer() const
	{ return _flags.IsSet(ftDoubleBuffer); }
	bool ClearBetweenFrames() const
	{ return _flags.IsSet(ftClearBF); }
	bool ScaleBetweenFrames() const
	{ return _flags.IsSet(ftScaleBF); }
	bool NewViewBetweenFrames() const
	{ return _flags.IsSet(ftNewViewBF); }
	bool HCenterBetweenFrames() const
	{ return _flags.IsSet(ftHCenterBF); }
	bool DisplayOnRequest() const
	{ return _flags.IsSet(ftDisplayOnRequest); }
	int SwapInterval() const
	{ return _SwapInterval; }
	int FirstFrame() const
	{ return _FirstFrame; }
	int LastFrame() const
	{ return _LastFrame; }
	int Step() const
	{ return _Step; }
	const char* Intervals() const
	{ return _Intervals; }
	bool IntervalsSet() const
	{ return 0 != _Intervals[0]; }

	void DoubleBuffer(bool v)
	{ _flags.Set(ftDoubleBuffer, v); }
	void ClearBetweenFrames(bool v)
	{ _flags.Set(ftClearBF, v); }
	void ScaleBetweenFrames(bool v)
	{ _flags.Set(ftScaleBF, v); }
	void NewViewBetweenFrames(bool v)
	{ _flags.Set(ftNewViewBF, v); }
	void HCenterBetweenFrames(bool v)
	{ _flags.Set(ftHCenterBF, v); }
	void DisplayOnRequest(bool v)
	{ _flags.Set(ftDisplayOnRequest, v); }

	void SwapInterval(int v)
	{ _SwapInterval = v; }
	void FirstFrame(int v)
	{ _FirstFrame = v; }
	void LastFrame(int v)
	{ _LastFrame = v; }
	void Step(int v)
	{ _Step = v; }
	void Intervals(const char* str)
	{ 
		strncpy(_Intervals, str, IntervalsLength-1); 
		_Intervals[IntervalsLength-1] = 0;
	}
	
	void Default();
	enum nf
	{
		nfConsecutive,
		nfStepNo
	};
	nf FrameNumbers() const
	{ return _FrameNumbers; }
	void FrameNumbers(nf Fn)
	{ _FrameNumbers = Fn; }
private:
	enum { IntervalsLength = 80 };
	enum FlagTags
	{
		ftDoubleBuffer     = 1 << 0,
		ftClearBF          = 1 << 1,
		ftScaleBF          = 1 << 2,
		ftNewViewBF        = 1 << 3,
		ftHCenterBF        = 1 << 4,
		ftDisplayOnRequest = 1 << 5
	};
	FlagSet _flags;

	nf _FrameNumbers;
	int _SwapInterval;
	int _FirstFrame;
	int _LastFrame; // -1 means default
	int _Step;
	char _Intervals[IntervalsLength];
};


#else
	#error File already included
#endif
