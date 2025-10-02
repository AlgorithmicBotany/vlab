#include <fstream>

#include <fw.h>

#include "lstudioptns.h"
#include "resource.h"

#include <browser/params.h>


const char* LStudioOptions::_CfgLabels[] =
{
	"Text editor font:",               //   0
		"Display on tabs:",            //   1
		"Grid bg:",                    //   2
		"Grid grid:",                  //   3
		"Grid axis:",                  //   4
		"Grid curve:",                 //   5
		"Grid segments:",              //   6
		"Grid points:",                //   7
		"Grid labels:",                //   8
		"Grid curve width:",           //   9
		"Grid segment width:",         //  10
		"Text editor colors:",         //  11
		"Oofs root:",                  //  12
		"Diff list:",                  //  13
		"Grid point size:",            //  14
		"Confirm exit:",               //  15
		"Experimental:",               //  16
		"External L-system editor:",   //  17
		"RA cr-lf:",                   //  18
		"Browser logon:",              //  19
		"Browser icon width:",         //  20
		"Browser font:",               //  21
		"Warn gallery delete:",        //  22
		"Browser background:",         //  23
		"Ignored files:",              //  24
		"Hide empty tabs:",            //  25
		"Clean storage before write:", //  26
		"Help command:",               //  27
		0
};



LStudioOptions options;

const int DefaultTabFontSize = 12;

LStudioOptions::LStudioOptions() : 
_editlf(9, "Fixedsys"),
_editFont(9, "Fixedsys")
{
	_flags.Set(ftExpired, false);
	_SetDefault();

	Font f(_editlf);
	ExchangeGDIobjects(f, _editFont);
	_hBgBrush = 0;
	_UpdateBgBrush();
}

LStudioOptions::~LStudioOptions()
{
	DeleteObject(_hBgBrush);
}



void LStudioOptions::_SetDefault()
{
	{
		_editlf.lfHeight = 9;
		_editlf.lfWidth = 0;
		_editlf.lfEscapement = 0;
		_editlf.lfOrientation = 0;
		_editlf.lfWeight = FW_DONTCARE;
		_editlf.lfItalic = false;
		_editlf.lfUnderline = false;
		_editlf.lfStrikeOut = false;
		_editlf.lfCharSet = DEFAULT_CHARSET;
		_editlf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		_editlf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		_editlf.lfQuality = DEFAULT_QUALITY;
		_editlf.lfPitchAndFamily = DEFAULT_PITCH;
		_tcscpy(_editlf.lfFaceName, __TEXT("Fixedsys"));
	}
	{
		_displayOnTabs = Options::dtText;
	}
	{
		_GridColors[Options::eBackground] = RGB(0, 0, 0);
		_GridColors[Options::eGrid] = RGB(102, 102, 102);
		_GridColors[Options::eAxis] = RGB(255, 255, 255);
		_GridColors[Options::eCurve] = RGB(255, 255, 0);
		_GridColors[Options::eSegments] = RGB(153, 153, 255);
		_GridColors[Options::ePoints] = RGB(255, 255, 255);
		_GridColors[Options::eLabels] = RGB(0, 255, 0);
		_UpdateFColors();
	}
	{
		SetCurveWidth(1.0f);
		SetSegmentsWidth(1.0f);
		SetPointSize(3.5f);
	}
	{
		_EditorColors.Txt = RGB(0, 0, 0);
		_EditorColors.Bckgnd = RGB(255, 255, 255);
	}

	{
		_OofsRoot[0] = 0;
	}
	{
		_flags.Set(ftDiffList, true);
		_flags.Set(ftConfirmExit, true);
		_flags.Set(ftExperimental, false);
		_flags.Set(ftExtLsysEdit, false);
		_flags.Set(ftWarnGalleryDelete, true);
		_flags.Set(ftHideEmptyTabs, false);
		_flags.Set(ftCleanStorageBeforeWrite, false);
	}
	{
		_crlf.reset();
		strcpy(_Host, "host");
		strcpy(_Oofs, "/path");
		strcpy(_User, "user");
		_Password[0] = 0;
		_IconWidth = VLB::Parameters::Params[VLB::Parameters::pObjIconX];
		_BrowserFontSize = VLB::Parameters::Params[VLB::Parameters::pDefFontSize];
		BrowserFontName =_DefFontName();
		_BrowserBg = RGB(0, 64, 0);
	}
	_ignored.reset();
	_HelpCommands.clear();
}



void LStudioOptions::SetEditorFont(const LogFont& lf)
{
	Font f(lf);
	ExchangeGDIobjects(_editFont, f);
}

void LStudioOptions::SetGridColors(const COLORREF* arr)
{
	for (int i=0; i<Options::eGridViewEntryCount; i++)
		_GridColors[i] = arr[i];
	_UpdateFColors();
}

void LStudioOptions::SetGridWidths(const float* arr)
{
	for (int i=0; i<Options::wGridWidthEntryCount; i++)
		_aGridWidth[i] = arr[i];
}

void LStudioOptions::SetOofsRoot(const std::string& oofsroot)
{
	_tcscpy(_OofsRoot, oofsroot.c_str());
}


void LStudioOptions::_UpdateFColors()
{
	for (int i=0; i<Options::eGridViewEntryCount; i++)
	{
		_fGridColors[i][0] = GetRValue(_GridColors[i])/255.0f;
		_fGridColors[i][1] = GetGValue(_GridColors[i])/255.0f;
		_fGridColors[i][2] = GetBValue(_GridColors[i])/255.0f;
	}
}


void LStudioOptions::SetTextEditBgColor(COLORREF clr)
{ 
	_EditorColors.Bckgnd = clr;
	_UpdateBgBrush();
}


void LStudioOptions::_UpdateBgBrush()
{
	if (0 != _hBgBrush)
		DeleteObject(_hBgBrush);
	_hBgBrush = CreateSolidBrush(_EditorColors.Bckgnd);
	if (0 == _hBgBrush)
	{
		_hBgBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
		_EditorColors.Bckgnd = RGB(255, 255, 255);
	}
}


void LStudioOptions::Save(std::ofstream& trg) const
{
	trg << _CfgLabels[lTextEditorFont] << ' '
		<< _editlf.lfHeight << ' '
		<< _editlf.lfWidth << ' '
		<< _editlf.lfEscapement << ' '
		<< _editlf.lfOrientation << ' '
		<< _editlf.lfWeight << ' '
		<< static_cast<int>(_editlf.lfItalic) << ' '
		<< static_cast<int>(_editlf.lfUnderline) << ' '
		<< static_cast<int>(_editlf.lfStrikeOut) << ' '
		<< static_cast<int>(_editlf.lfCharSet) << ' '
		<< static_cast<int>(_editlf.lfOutPrecision) << ' '
		<< static_cast<int>(_editlf.lfClipPrecision) << ' '
		<< static_cast<int>(_editlf.lfQuality) << ' '
		<< static_cast<int>(_editlf.lfPitchAndFamily) << ' '
		<< _editlf.lfFaceName << std::endl;

	trg << _CfgLabels[lDisplayOnTabs] << ' ';
	switch (_displayOnTabs)
	{
	case Options::dtText :
		trg << "text";
		break;
	case Options::dtIcons :
		trg << "icons";
		break;
	case Options::dtBoth :
		trg << "both";
		break;
	}
	trg << std::endl;

	trg << _CfgLabels[lGridBg] << ' '
		<< static_cast<int>(GetRValue(_GridColors[Options::eBackground])) << ' '
		<< static_cast<int>(GetGValue(_GridColors[Options::eBackground])) << ' '
		<< static_cast<int>(GetBValue(_GridColors[Options::eBackground])) << std::endl;

	trg << _CfgLabels[lGridGrid] << ' '
		<< static_cast<int>(GetRValue(_GridColors[Options::eGrid])) << ' '
		<< static_cast<int>(GetGValue(_GridColors[Options::eGrid])) << ' '
		<< static_cast<int>(GetBValue(_GridColors[Options::eGrid])) << std::endl;

	trg << _CfgLabels[lGridAxis] << ' '
		<< static_cast<int>(GetRValue(_GridColors[Options::eAxis])) << ' '
		<< static_cast<int>(GetGValue(_GridColors[Options::eAxis])) << ' '
		<< static_cast<int>(GetBValue(_GridColors[Options::eAxis])) << std::endl;

	trg << _CfgLabels[lGridCurve] << ' '
		<< static_cast<int>(GetRValue(_GridColors[Options::eCurve])) << ' '
		<< static_cast<int>(GetGValue(_GridColors[Options::eCurve])) << ' '
		<< static_cast<int>(GetBValue(_GridColors[Options::eCurve]))  << std::endl;

	trg << _CfgLabels[lGridSegments] << ' '
		<< static_cast<int>(GetRValue(_GridColors[Options::eSegments])) << ' '
		<< static_cast<int>(GetGValue(_GridColors[Options::eSegments])) << ' '
		<< static_cast<int>(GetBValue(_GridColors[Options::eSegments])) << std::endl;

	trg << _CfgLabels[lGridPoints] << ' '
		<< static_cast<int>(GetRValue(_GridColors[Options::ePoints])) << ' '
		<< static_cast<int>(GetGValue(_GridColors[Options::ePoints])) << ' '
		<< static_cast<int>(GetBValue(_GridColors[Options::ePoints])) << std::endl;

	trg << _CfgLabels[lGridLabels] << ' '
		<< static_cast<int>(GetRValue(_GridColors[Options::eLabels])) << ' '
		<< static_cast<int>(GetGValue(_GridColors[Options::eLabels])) << ' '
		<< static_cast<int>(GetBValue(_GridColors[Options::eLabels])) << std::endl;

	trg << _CfgLabels[lGridCurveWidth] << ' ' << GetCurveWidth() << std::endl;

	trg << _CfgLabels[lGridSegmentWidth] << ' ' <<  GetSegmentsWidth() << std::endl;

	trg <<  _CfgLabels[lTextEditorColors] << ' '
		<< static_cast<int>(GetRValue(_EditorColors.Txt)) << ' '
		<< static_cast<int>(GetGValue(_EditorColors.Txt)) << ' '
		<< static_cast<int>(GetBValue(_EditorColors.Txt)) << ' '
		<< static_cast<int>(GetRValue(_EditorColors.Bckgnd)) << ' '
		<< static_cast<int>(GetGValue(_EditorColors.Bckgnd)) << ' '
		<< static_cast<int>(GetBValue(_EditorColors.Bckgnd)) << std::endl;

	trg << _CfgLabels[lOofsRoot] << ' ' << _OofsRoot << std::endl;

	trg << _CfgLabels[lDiffList] << ' ';
	if (UseDiffList())
		trg << "on";
	else
		trg << "off";
	trg << std::endl;

	trg << _CfgLabels[lConfirmExit] << ' ';
	if (ConfirmExit())
		trg << "on";
	else
		trg << "off";
	trg << std::endl;

	trg << _CfgLabels[lGridPointSize] << ' ' << GetPointSize() << std::endl;

	trg << _CfgLabels[lExperimental] << ' ';
	if (Experimental())
		trg << "on";
	else
		trg << "off";
	trg << std::endl;

	if (ExternalLsysEdit())
		trg << _CfgLabels[lExternalLsystemEditor] << " on" << std::endl;

	if (!_crlf.is_empty())
	{
		trg << _CfgLabels[lRAcrlf];
		size_t pos = _crlf.begin();
		while (pos != string_buffer::npos)
		{
			trg << ' ' << _crlf.string(pos);
			pos = _crlf.find_next(pos);
		}
		trg << std::endl;
	}

	if (0 != _Host[0])
	{
		trg << _CfgLabels[lBrowserLogon] << ' ' << _Host;
		if (0 != _Oofs[0])
		{
			trg << ' ' << _Oofs;
			if (0 != _User[0])
			{
				trg << ' ' << _User;
				if (0 != _Password[0])
					trg << ' ' << _Password;
			}
		}
		trg << std::endl;
	}

	trg << _CfgLabels[lBrowserIconWidth] << ' ' << _IconWidth << std::endl;

	trg << _CfgLabels[lBrowserFont] << " \"" << BrowserFontName << "\" " << _BrowserFontSize << std::endl;

	if (!WarnGalleryDelete())
		trg << _CfgLabels[lWarnGalleryDelete] << " off" << std::endl;

	trg << _CfgLabels[lBrowserBackground] << ' '
		<< static_cast<int>(GetRValue(_BrowserBg)) << ' '
		<< static_cast<int>(GetGValue(_BrowserBg)) << ' '
		<< static_cast<int>(GetBValue(_BrowserBg)) << std::endl;

	if (!_ignored.is_empty())
	{
		trg << _CfgLabels[lIgnoredFiles];
		for (string_buffer::const_iterator it(_ignored); !it.at_end(); it.advance())
			trg << ' ' << it.str();
		trg << std::endl;
	}

	trg << _CfgLabels[lCleanStorageBeforeWrite] << ' ';
	if (CleanStorageBeforeWrite())
		trg << "on";
	else
		trg << "off";
	trg << std::endl;

	for (HelpCommandCItem item = _HelpCommands.begin(); item != _HelpCommands.end(); ++item)
	{
		trg << _CfgLabels[lHelpCommand] << ' ';
		trg << '\"' << item->mLabel << "\" " << item->mCommand << std::endl;
	}
}


void LStudioOptions::_LoadLogFont(const char* bf, const ReadTextFile& src)
{
	int h, wd, e, o, wg, i, u, s, c, op, cl, q, pf;
	char fnm[LF_FACESIZE];
	int res = sscanf
		(
		bf, "%d %d %d %d %d %d %d %d %d %d %d %d %d",
		&h, &wd, &e, &o, &wg, &i, &u, &s, &c, &op, &cl, &q, &pf
		);
	if (res != 13)
		throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
	while (isspace(*bf) || isdigit(*bf))
		bf++;
	strcpy(fnm, bf);
	res = strlen(fnm) - 1;
	while (isspace(fnm[res]))
		fnm[res--] = 0;

	LOGFONT lf;
	lf.lfHeight = h;
	lf.lfWidth = wd;
	lf.lfEscapement = e;
	lf.lfOrientation = o;
	lf.lfWeight = wg;
	lf.lfItalic = static_cast<BYTE>(i);
	lf.lfUnderline = static_cast<BYTE>(u);
	lf.lfStrikeOut = static_cast<BYTE>(s);
	lf.lfCharSet = static_cast<BYTE>(c);
	lf.lfOutPrecision = static_cast<BYTE>(op);
	lf.lfClipPrecision = static_cast<BYTE>(cl);
	lf.lfQuality = static_cast<BYTE>(q);
	lf.lfPitchAndFamily = static_cast<BYTE>(pf);
	strcpy(lf.lfFaceName, fnm);

	SetEditorFont(lf);
}


COLORREF LStudioOptions::_LoadColor(const char* ln, const ReadTextFile& src) const
{
	int r, g, b;
	int res = sscanf(ln, "%d %d %d", &r, &g, &b);
	if (res != 3)
		throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
	return RGB(r, g, b);
}

void LStudioOptions::_LoadDisplayOnTabs(const char* ln, const ReadTextFile& src)
{
	char dp[12];
	int res = sscanf(ln, "%10s", dp);
	if (1 != res)
		throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
	if (!(strcmp(dp, "text")))
		_displayOnTabs = Options::dtText;
	else if (!(strcmp(dp, "icons")))
		_displayOnTabs = Options::dtIcons;
	else if (!(strcmp(dp, "both")))
		_displayOnTabs = Options::dtBoth;
	else
		throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
}

void LStudioOptions::Load(ReadTextFile& src)
{
	_SetDefault();
	std::string ln;
	while (!src.Eof())
	{
		src.Read(ln);
		if (ln.empty())
			continue;
		int l = 0;
		int i= _Label(ln, l, _CfgLabels, -1);
		ln.erase(0, l);
		switch (i)
		{
		case lTextEditorFont :
			_LoadLogFont(ln.c_str(), src);
			break;
		case lDisplayOnTabs :
			_LoadDisplayOnTabs(ln.c_str(), src);
			break;
		case lGridBg :
			{
				COLORREF rgb = _LoadColor(ln.c_str(), src);
				_GridColors[Options::eBackground] = rgb;
			}
			break;
		case lGridGrid :
			{
				COLORREF rgb = _LoadColor(ln.c_str(), src);
				_GridColors[Options::eGrid] = rgb;
			}
			break;
		case lGridAxis :
			{
				COLORREF rgb = _LoadColor(ln.c_str(), src);
				_GridColors[Options::eAxis] = rgb;
			}
			break;
		case lGridCurve :
			{
				COLORREF rgb = _LoadColor(ln.c_str(), src);
				_GridColors[Options::eCurve] = rgb;
			}
			break;
		case lGridSegments :
			{
				COLORREF rgb = _LoadColor(ln.c_str(), src);
				_GridColors[Options::eSegments] = rgb;
			}
			break;
		case lGridPoints :
			{
				COLORREF rgb = _LoadColor(ln.c_str(), src);
				_GridColors[Options::ePoints] = rgb;
			}
			break;
		case lGridLabels :
			{
				COLORREF rgb = _LoadColor(ln.c_str(), src);
				_GridColors[Options::eLabels] = rgb;
			}
			break;
		case lGridCurveWidth :
			{
				int res = sscanf(ln.c_str(), "%f", &_aGridWidth[Options::wCurveWidth]);
				if (res != 1)
					throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
			}
			break;
		case lGridSegmentWidth :
			{
				int res = sscanf(ln.c_str(), "%f", &_aGridWidth[Options::wSegmentWidth]);
				if (res != 1)
					throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
			}
			break;
		case lTextEditorColors :
			{
				int rt, gt, bt, rb, gb, bb;
				int res = sscanf(ln.c_str(), "%d %d %d %d %d %d", &rt, &gt, &bt, &rb, &gb, &bb);
				if (res != 6)
					throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
				_EditorColors.Txt = RGB(rt, gt, bt);
				_EditorColors.Bckgnd = RGB(rb, gb, bb);
				_UpdateBgBrush();
			}
			break;
		case lOofsRoot :
			{
				while (isspace(ln[l]))
					l++;
				strncpy(_OofsRoot, ln.c_str(), _MAX_PATH);
				_OofsRoot[_MAX_PATH] = 0;
			}
			break;
		case lDiffList :
			_flags.Set(ftDiffList, LoadOnOff(ln, src));
			break;
		case lGridPointSize :
			{
				float v;
				int res = sscanf(ln.c_str(), "%f", &v);
				if (res != 1)
					throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
				SetPointSize(v);
			}
			break;
		case lConfirmExit :
			_flags.Set(ftConfirmExit, LoadOnOff(ln, src));
			break;
		case lExperimental :
			_flags.Set(ftExperimental, LoadOnOff(ln, src));
			break;
		case lExternalLsystemEditor :
			_flags.Set(ftExtLsysEdit, LoadOnOff(ln, src));
			break;
		case lRAcrlf :
			_LoadCRLF(ln.c_str());
			break;
		case lBrowserLogon :
			_LoadLogon(ln.c_str());
			break;
		case lBrowserIconWidth :
			_LoadIconWidth(ln.c_str());
			break;
		case lBrowserFont :
			_LoadBrFont(ln.c_str());
			break;
		case lWarnGalleryDelete :
			_flags.Set(ftWarnGalleryDelete, LoadOnOff(ln, src));
			break;
		case lBrowserBackground :
			_BrowserBg = _LoadColor(ln.c_str(), src);
			break;
		case lIgnoredFiles :
			_LoadIgnored(ln.c_str()+l);
			break;
		case lHideEmptyTabs :
			_flags.Set(ftHideEmptyTabs, LoadOnOff(ln, src));
			break;
		case lCleanStorageBeforeWrite :
			_flags.Set(ftCleanStorageBeforeWrite, LoadOnOff(ln, src));
			break;
		case lHelpCommand:
			_LoadHelpCommand(ln.c_str());
			break;
		case eComment:
		case eEmptyLine:
			break;
		default :
			throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
		}
	}
	_UpdateFColors();
}


bool LStudioOptions::LoadOnOff(const std::string& ln, ReadTextFile& src) const
{
	const int BfSize = 10;
	char str[BfSize];
	int res = sscanf(ln.c_str(), "%s", str);
	if (res != 1)
		throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
	if (!(strcmp(str, "on")))
		return true;
	else if (!(strcmp(str, "off")))
		return false;
	throw Exception(IDERR_READINGCONFIG, src.Filename(), src.Line());
}


void LStudioOptions::_LoadCRLF(const char* ln)
{
	ln = SkipBlanks(ln);
	while (0 != ln[0])
	{
		while (!isspace(ln[0]) && 0 != ln[0])
			ln = _crlf.add_str(ln);
		ln = SkipBlanks(ln);
	}
}

void LStudioOptions::_LoadLogon(const char* ln)
{
	ln = SkipBlanks(ln);
	const int FmtSz = 64;
	char fmt[FmtSz];
	sprintf(fmt, "%%%ds %%%ds %%%ds %%%ds", MaxHostLength, MaxOofsLength, MaxUserLength, MaxPasswordLength);
	sscanf(ln, fmt, _Host, _Oofs, _User, _Password);
}

void LStudioOptions::_LoadIconWidth(const char* ln)
{
	ln = SkipBlanks(ln);
	int iw = atoi(ln);
	if (iw<MinIconWidth)
		iw = MinIconWidth;
	else if (iw>MaxIconWidth)
		iw = MaxIconWidth;
	while (0 != (iw %4))
		--iw;
	_IconWidth = iw;
}

void LStudioOptions::_LoadBrFont(const char* ln)
{
	ln = SkipBlanks(ln);
	ln = ReadQuotedString(ln, BrowserFontName);
	ln = SkipBlanks(ln);
	int fs = atoi(ln);
	if (fs<MinFontSize)
		fs = MinFontSize;
	else if (fs>MaxFontSize)
		fs = MaxFontSize;
	_BrowserFontSize = fs;
}

void LStudioOptions::_LoadIgnored(const char* ln)
{
	ln = SkipBlanks(ln);
	while (ln[0] != 0)
	{
		ln = _ignored.add_str(ln);
		ln = SkipBlanks(ln);
	}
}

bool LStudioOptions::ConvertCRLF(const char* fname) const
{
	if (_crlf.contains(fname))
		return true;
	const char* ext = strrchr(fname, '.');
	if (0 == ext)
		return false;
	else
		return _crlf.contains(ext+1);
}


void LStudioOptions::_LoadHelpCommand(const char* line)
{
	std::string label;
	// find \"
	line = SkipBlanks(line);
	if (NULL == line || *line != '\"')
		return;

	line = ReadQuotedString(line, label);
	if (NULL == line)
		return;
	line = SkipBlanks(line);
	if (line == NULL || *line == 0)
		return;

	std::string command(line);

	HelpCommand helpCommand;
	helpCommand.mLabel = label;
	helpCommand.mCommand = command;

	_HelpCommands.push_back(helpCommand);
}

