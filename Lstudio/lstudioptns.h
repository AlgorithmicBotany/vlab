#ifndef __LSTUDIOOPTIONS_H__
#define __LSTUDIOOPTIONS_H__


namespace Options
{
	enum DisplayOnTabs
	{
		dtText = 0,
			dtIcons,
			dtBoth
	};
	enum GridViewElements
	{
		eBackground = 0,
			eGrid,
			eAxis,
			eCurve,
			eSegments,
			ePoints,
			eLabels,
			eGridViewEntryCount
	};
	enum GridWidth
	{ 
		wCurveWidth = 0, 
			wSegmentWidth,
			wPointSize,
			wGridWidthEntryCount
	};
}

class LStudioOptions : ConfigFile
{
public:
	LStudioOptions();
	~LStudioOptions();

	void Save(std::ofstream&) const;
	void Load(ReadTextFile&);

	const LogFont& GetLogFont() const
	{ return _editlf; }
	void SetEditorFont(const LogFont&);
	const Font& EditorFont() const
	{ return _editFont; }
	Options::DisplayOnTabs GetDisplayOnTabs() const
	{ return _displayOnTabs; }
	void SetDisplayOnTabs(Options::DisplayOnTabs dot)
	{ _displayOnTabs = dot; }
	void SetGridColors(const COLORREF*);
	void SetGridWidths(const float*);
	const float* GetGridWidths() const
	{ return _aGridWidth; }
	const COLORREF* GetGridColors() const
	{ return _GridColors; }
	const float* GetGridColor(Options::GridViewElements e) const
	{
		assert(e != Options::eGridViewEntryCount);
		return _fGridColors[e];
	}
	COLORREF GetTextEditTxtColor() const
	{ return _EditorColors.Txt; }
	COLORREF GetTextEditBgColor() const
	{ return _EditorColors.Bckgnd; }
	void SetTextEditTxtColor(COLORREF clr)
	{ _EditorColors.Txt = clr; }
	void SetTextEditBgColor(COLORREF);
	HBRUSH GetTextEditBgBrush() const
	{ return _hBgBrush; }
	float GetSegmentsWidth() const
	{ return _aGridWidth[Options::wSegmentWidth]; }
	void SetSegmentsWidth(float v)
	{ _aGridWidth[Options::wSegmentWidth] = v; }
	float GetCurveWidth() const
	{ return _aGridWidth[Options::wCurveWidth]; }
	void SetCurveWidth(float v)
	{ _aGridWidth[Options::wSegmentWidth] = v; }
	float GetPointSize() const
	{ return _aGridWidth[Options::wPointSize]; }
	void SetPointSize(float v)
	{ _aGridWidth[Options::wPointSize] = v; }
	const TCHAR* OofsRoot() const
	{ return _OofsRoot; }
	void SetOofsRoot(const std::string&);
	void Reset()
	{ _SetDefault(); }
	bool UseDiffList() const
	{ return _flags.IsSet(ftDiffList); }
	bool ConfirmExit() const
	{ return _flags.IsSet(ftConfirmExit); }
	bool Experimental() const
	{ return _flags.IsSet(ftExperimental); }
	bool ExternalLsysEdit() const
	{ return _flags.IsSet(ftExtLsysEdit); }
	bool WarnGalleryDelete() const
	{ return _flags.IsSet(ftWarnGalleryDelete); }
	const std::string& GetBrowserFontName() const
	{ return BrowserFontName; }
	int BrowserFontSize() const
	{ return _BrowserFontSize; }
	const char* Host() const
	{ return _Host; }
	void Host(const char* host)
	{ strncpy(_Host, host, MaxHostLength); _Host[MaxHostLength] = 0; }
	const char* Oofs() const
	{ return _Oofs; }
	void Oofs(const char* oofs)
	{ strncpy(_Oofs, oofs, MaxOofsLength); _Oofs[MaxOofsLength] = 0; }
	const char* User() const
	{ return _User; }
	void User(const char* user)
	{ strncpy(_User, user, MaxUserLength); _User[MaxUserLength] = 0; }
	const char* Password() const
	{ return _Password; }
	void Password(const char* password)
	{ strncpy(_Password, password, MaxPasswordLength); _Password[MaxPasswordLength] = 0; }
	bool ConvertCRLF(const char*) const;
	const string_buffer& ConvertCRLF() const
	{ return _crlf; }
	int IconWidth() const
	{ return _IconWidth; }
	COLORREF BrowserBg() const
	{ return _BrowserBg; }
	const string_buffer& IgnoredFiles() const
	{ return _ignored; }
	bool Expired() const
	{ return _flags.IsSet(ftExpired); }
	void Expired(bool f)
	{ _flags.Set(ftExpired, f); }
	void HideEmptyTabs(bool f)
	{ _flags.Set(ftHideEmptyTabs, f); }
	bool HideEmptyTabs() const
	{ return _flags.IsSet(ftHideEmptyTabs); }
	bool CleanStorageBeforeWrite() const
	{ return _flags.IsSet(ftCleanStorageBeforeWrite); }

	struct HelpCommand
	{
		std::string mLabel;
		std::string mCommand;
	};

	int GetHelpCommandCount() const
	{ return _HelpCommands.size(); }

	const HelpCommand& GetHelpCommand(int iItem) const
	{
		assert(iItem>=0);
		assert(iItem<GetHelpCommandCount());
		return _HelpCommands[iItem];
	}
private:

	void _LoadLogFont(const char* bf, const ReadTextFile&);
	void _LoadDisplayOnTabs(const char*, const ReadTextFile&);
	COLORREF _LoadColor(const char*, const ReadTextFile&) const;
	void _LoadCRLF(const char*);
	void _LoadLogon(const char*);
	void _LoadIconWidth(const char*);
	void _LoadBrFont(const char*);
	void _LoadIgnored(const char*);
	bool LoadOnOff(const std::string& line, ReadTextFile& srcFile) const;
	void _LoadHelpCommand(const char* line);

	const char* _DefFontName() const
	{ return "Arial"; }

	//static const char* _SkipBlanks(const char*);
	//const char* _ReadQuotedString(const char*, char*, int) const;
	//const char* _ReadQuotedString(const char* line, std::string& quotedString) const;

	void _SetDefault();
	void _UpdateFColors();
	void _UpdateBgBrush();

	enum cnsts
	{
		MinIconWidth = 16,
		MaxIconWidth = 256,
		MaxHostLength = 64,
		MaxOofsLength = 128,
		MaxUserLength = 64,
		MaxPasswordLength = 64,
		MinFontSize = 8,
		MaxFontSize = 92,
		MaxFontLength = 48
	};

	enum labels
	{
		lTextEditorFont = 0,
		lDisplayOnTabs,
		lGridBg,
		lGridGrid,
		lGridAxis,
		lGridCurve,
		lGridSegments,
		lGridPoints,
		lGridLabels,
		lGridCurveWidth,
		lGridSegmentWidth,
		lTextEditorColors,
		lOofsRoot,
		lDiffList,
		lGridPointSize,
		lConfirmExit,
		lExperimental,
		lExternalLsystemEditor,
		lRAcrlf,
		lBrowserLogon,
		lBrowserIconWidth,
		lBrowserFont,
		lWarnGalleryDelete,
		lBrowserBackground,
		lIgnoredFiles,
		lHideEmptyTabs,
		lCleanStorageBeforeWrite,
		lHelpCommand,

	};


	LogFont _editlf;
	Font _editFont;
	HBRUSH _hBgBrush;
	Options::DisplayOnTabs _displayOnTabs;
	COLORREF _GridColors[Options::eGridViewEntryCount];
	float _fGridColors[Options::eGridViewEntryCount][3];
	float _aGridWidth[Options::wGridWidthEntryCount];
	struct TextEditorColors
	{
		COLORREF Txt;
		COLORREF Bckgnd;
	} _EditorColors;
	TCHAR _OofsRoot[_MAX_PATH+1];  // for the local browser

	enum FlagTags
	{
		ftDiffList                = 1 << 0,
		ftConfirmExit             = 1 << 1,
		ftExperimental            = 1 << 2,
		ftExtLsysEdit             = 1 << 3,
		ftWarnGalleryDelete       = 1 << 4,
		ftExpired                 = 1 << 5,
		ftHideEmptyTabs           = 1 << 6,
		ftCleanStorageBeforeWrite = 1 << 7
	};
	FlagSet _flags;
	string_buffer _crlf;
	char _Host[MaxHostLength+1];
	char _Oofs[MaxOofsLength+1];
	char _User[MaxUserLength+1];
	char _Password[MaxPasswordLength+1];
	//char _BrowserFontName[MaxFontLength+1];
	std::string BrowserFontName;
	int _IconWidth;
	int _BrowserFontSize;
	COLORREF _BrowserBg;
	string_buffer _ignored;
	std::vector<HelpCommand> _HelpCommands;
	typedef std::vector<HelpCommand>::const_iterator HelpCommandCItem;

	static const char* _CfgLabels[];
};


extern LStudioOptions options;


#endif
