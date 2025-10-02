#include <fw.h>

#include "animdata.h"
#include "animedit.h"

#include "resource.h"


INIT_COUNTER(AnimateEdit);

static const char* AnimLines[] = 
{
	"double buffer:",                 //  0
		"clear between frames:",      //  1
		"scale between frames:",      //  2
		"new view between frames:",   //  3
		"swap interval:",             //  4
		"first frame:",               //  5
		"last frame:",                //  6
		"step:",                      //  7
		"frame intervals:",           //  8
		"hcenter between frames:",    //  9
		"display on request:",        // 10
		"frame numbers:"              // 11
};

enum AnimTags
{
	tDoubleBuffer = 0,
		tClearBetween,
		tScaleBetween,
		tNewViewBetween,
		tSwapInterval,
		tFirstFrame,
		tLastFrame,
		tStep,
		tFrameIntervals,
		tHCenterBetween,
		tDisplayOnRequest,
		tFrameNumbers
};


const int TagsCount = CountOf(AnimLines);

AnimateEdit::AnimateEdit(HWND hwnd) : FormCtrl(hwnd),
_Last(GetDlgItem(IDC_LASTFRAME))
{
	_Filename[0] = 0;

	_UpdateControls();
	INC_COUNTER;
}


void AnimateEdit::_UpdateControls() const
{
	// Double buffer
	{
		Button Double(GetDlgItem(IDC_DOUBLEBUFFER));
		Double.SetCheck(_data.DoubleBuffer());
	}
	// First frame, step, interval
	{
		EditLine FirstFrame(GetDlgItem(IDC_FIRSTFRAME));
		FirstFrame.SetInt(_data.FirstFrame());
	}
	{
		EditLine Step(GetDlgItem(IDC_STEP));
		Step.SetInt(_data.Step());
	}
	{
		EditLine SwapInterval(GetDlgItem(IDC_SWAPINTERVAL));
		SwapInterval.SetInt(_data.SwapInterval());
	}
	{
		Button LastDefault(GetDlgItem(IDC_LASTDEFAULT));
		LastDefault.SetCheck(-1 == _data.LastFrame());
	}
	{
		Button LastSpecified(GetDlgItem(IDC_LASTSPECIFIED));
		LastSpecified.SetCheck(-1 != _data.LastFrame());
		_Last.Enable(-1 != _data.LastFrame());
		if (-1 != _data.LastFrame())
			SetDlgItemInt(Hwnd(), IDC_LASTFRAME, _data.LastFrame(), true);
	}
	{
		Button ClearBF(GetDlgItem(IDC_CLEARBF));
		ClearBF.SetCheck(_data.ClearBetweenFrames());
	}
	{
		Button ScaleBF(GetDlgItem(IDC_SCALEBF));
		ScaleBF.SetCheck(_data.ScaleBetweenFrames());
	}
	{
		Button NewViewBF(GetDlgItem(IDC_NEWVIEWBF));
		NewViewBF.SetCheck(_data.NewViewBetweenFrames());
	}
	{
		Button HCenterBF(GetDlgItem(IDC_HCENTERBF));
		HCenterBF.SetCheck(_data.HCenterBetweenFrames());
	}
	{
		Button DispOnRequest(GetDlgItem(IDC_DISPONREQUEST));
		DispOnRequest.SetCheck(_data.DisplayOnRequest());
	}
	{
		Button NumConsec(GetDlgItem(IDC_NUMCONSEC));
		NumConsec.SetCheck(_data.FrameNumbers() == AnimData::nfConsecutive);
	}
	{
		Button NumStepNo(GetDlgItem(IDC_STEPNO));
		NumStepNo.SetCheck(_data.FrameNumbers() == AnimData::nfStepNo);
	}
	{
		EditLine Intervals(GetDlgItem(IDC_FRAMEINTERVALS));
		Intervals.SetText(_data.Intervals());
	}

}

AnimateEdit::~AnimateEdit()
{
	DEC_COUNTER;
}




HWND AnimateEdit::Create(HWND hParent, HINSTANCE hInst)
{
	FormCreator<AnimateEdit> creator;
	return CreateDialogParam
		(
		hInst,
		MAKEINTRESOURCE(IDD_ANIMEDIT),
		hParent,
		reinterpret_cast<DLGPROC>(FormCtrl::DlgProc),
		reinterpret_cast<LPARAM>(&creator)
		);
}




bool AnimateEdit::Command(int id, Window ctl, UINT)
{
	switch (id)
	{
	case IDC_FIRSTFRAME :
		{
			int i = ctl.GetInt();
			_data.FirstFrame(i);
		}
		break;
	case IDC_LASTDEFAULT :
		_Last.Enable(false);
		_data.LastFrame(-1);
		break;
	case IDC_LASTSPECIFIED :
		_Last.Enable();
		// Fall through
	case IDC_LASTFRAME :
		{
			int i = _Last.GetInt();
			_data.LastFrame(i);
		}
		break;
	case IDC_DOUBLEBUFFER :
		{
			Button DblBf(ctl);
			_data.DoubleBuffer(DblBf.IsChecked());
		}
		break;
	case IDC_STEP :
		{
			int i = ctl.GetInt();
			_data.Step(i);
		}
		break;
	case IDC_SWAPINTERVAL :
		{
			int i = ctl.GetInt();
			_data.SwapInterval(i);
		}
		break;
	case IDC_CLEARBF :
		{
			Button ClearBF(ctl);
			_data.ClearBetweenFrames(ClearBF.IsChecked());
		}
		break;
	case IDC_SCALEBF :
		{
			Button ScaleBF(ctl);
			_data.ScaleBetweenFrames(ScaleBF.IsChecked());
		}
		break;
	case IDC_NEWVIEWBF :
		{
			Button NewViewBF(ctl);
			_data.NewViewBetweenFrames(NewViewBF.IsChecked());
		}
		break;
	case IDC_HCENTERBF :
		{
			Button HCenterBF(ctl);
			_data.HCenterBetweenFrames(HCenterBF.IsChecked());
		}
		break;
	case IDC_FRAMEINTERVALS :
		{
			std::string bf;
			ctl.GetText(bf);
			_data.Intervals(bf.c_str());
		}
		break;
	case IDC_DISPONREQUEST :
		{
			Button DispOnReq(ctl);
			_data.DisplayOnRequest(DispOnReq.IsChecked());
		}
		break;
	case IDC_NUMCONSEC :
		{
			Button NumConsec(ctl);
			if (NumConsec.IsChecked())
				_data.FrameNumbers(AnimData::nfConsecutive);
			else
				_data.FrameNumbers(AnimData::nfStepNo);
		}
		break;
	case IDC_STEPNO :
		{
			Button StepNo(ctl);
			if (StepNo.IsChecked())
				_data.FrameNumbers(AnimData::nfStepNo);
			else
				_data.FrameNumbers(AnimData::nfConsecutive);
		}
		break;
	}
	return true;
}



const TCHAR* AnimateEdit::Name() const
{
	if (0 != _Filename[0])
		return _Filename;
	else
		return __TEXT("anim.a");
}


void AnimateEdit::Generate() const
{
	
	WriteTextFile trg(Name());

	trg.PrintF("%s %s\n", AnimLines[0], _data.DoubleBuffer() ? __TEXT("on") : __TEXT("off"));
	trg.PrintF("%s %s\n", AnimLines[1], _data.ClearBetweenFrames() ? __TEXT("on") : __TEXT("off"));
	trg.PrintF("%s %s\n", AnimLines[2], _data.ScaleBetweenFrames() ? __TEXT("on") : __TEXT("off"));
	trg.PrintF("%s %s\n", AnimLines[3], _data.NewViewBetweenFrames() ? __TEXT("on") : __TEXT("off"));
	trg.PrintF("%s %s\n", AnimLines[9], _data.HCenterBetweenFrames() ? __TEXT("on") : __TEXT("off"));
	trg.PrintF("%s %d\n", AnimLines[4], _data.SwapInterval());
	trg.PrintF("%s %d\n", AnimLines[5], _data.FirstFrame());
	trg.PrintF("%s %d\n", AnimLines[6], _data.LastFrame());
	trg.PrintF("%s %d\n", AnimLines[7], _data.Step());
	if (_data.IntervalsSet())
		trg.PrintF("%s %s\n", AnimLines[8], _data.Intervals());
	trg.PrintF("%s %s\n", AnimLines[10], _data.DisplayOnRequest() ? "on" : "off");
	trg.PrintF("%s %s\n", AnimLines[11], _data.FrameNumbers() == AnimData::nfConsecutive ? "consecutive" : "stepno");
}


void AnimateEdit::Clear()
{
	_Filename[0] = 0;
	_data.Default();
	_UpdateControls();
	{
		EditLine Fname(GetDlgItem(IDC_FNAME));
		Fname.SetText(Name());
	}
}


void AnimateEdit::Import(const TCHAR* fname)
{
	ReadTextFile src(fname);

	std::string line;

	for (;;)
	{
		src.Read(line);
		_InterpretLine(line.c_str());
		if (src.Eof())
			break;
	}

	_UpdateControls();
	_tcscpy(_Filename, fname);
	{
		EditLine Fname(GetDlgItem(IDC_FNAME));
		Fname.SetText(_Filename);
	}
}



void AnimateEdit::Export() const
{
	Generate();
}


void AnimateEdit::_InterpretLine(const char* line)
{
	assert(TagsCount-1 == tFrameNumbers);
	for (int i=0; i<TagsCount; i++)
	{
		if (!strncmp(AnimLines[i], line, strlen(AnimLines[i])))
		{
			switch (i)
			{
			case tDoubleBuffer :
				if (!strncmp(line+strlen(AnimLines[i])+1, "on", strlen("on")))
					_data.DoubleBuffer(true);
				else
					_data.DoubleBuffer(false);
				break;
			case tClearBetween :
				if (!strncmp(line+strlen(AnimLines[i])+1, "on", strlen("on")))
					_data.ClearBetweenFrames(true);
				else
					_data.ClearBetweenFrames(false);
				break;
			case tScaleBetween :
				if (!strncmp(line+strlen(AnimLines[i])+1, "on", strlen("on")))
					_data.ScaleBetweenFrames(true);
				else
					_data.ScaleBetweenFrames(false);
				break;
			case tNewViewBetween :
				if (!strncmp(line+strlen(AnimLines[i])+1, "on", strlen("on")))
					_data.NewViewBetweenFrames(true);
				else
					_data.NewViewBetweenFrames(false);
				break;
			case tHCenterBetween :
				if (!strncmp(line+strlen(AnimLines[i])+1, "on", strlen("on")))
					_data.HCenterBetweenFrames(true);
				else
					_data.HCenterBetweenFrames(false);
				break;
			case tSwapInterval :
				_data.SwapInterval(atoi(line+strlen(AnimLines[i])+1));
				break;
			case tFirstFrame :
				_data.FirstFrame(atoi(line+strlen(AnimLines[i])+1));
				break;
			case tLastFrame :
				_data.LastFrame(atoi(line+strlen(AnimLines[i])+1));
				break;
			case tStep :
				_data.Step(atoi(line+strlen(AnimLines[i])+1));
				break;
			case tFrameIntervals :
				_data.Intervals(line+strlen(AnimLines[i])+1);
				break;
			case tDisplayOnRequest :
				if (!strncmp(line+strlen(AnimLines[i])+1, "on", strlen("on")))
					_data.DisplayOnRequest(true);
				else
					_data.DisplayOnRequest(false);
				break;
			case tFrameNumbers :
				if (!strncmp(line+strlen(AnimLines[i])+1, "consecutive", strlen("consecutive")))
					_data.FrameNumbers(AnimData::nfConsecutive);
				else
					_data.FrameNumbers(AnimData::nfStepNo);
				break;
			default :
				{
					TCHAR bf[256];
					OemToChar(line, bf);
					MessageBox(IDERR_UNKNOWNANIMLABEL, bf);
				}
			}
		}
	}
}

