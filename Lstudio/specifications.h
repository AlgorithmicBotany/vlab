#ifndef __SPECIFICATIONS_H__
#define __SPECIFICATIONS_H__


class Specifications : ConfigFile
{
public:

	enum eProjectMode
	{
		mLsystem = 0,
		mView,
		mAnimate,
		mColors,
		mSurface,
		mContours,
		mFunctions,
		mPanels,
		mDescription,
		mAnyText,

		mUnspecified
	};

	enum eModelType
	{
		mtCpfg = 0,
		mtLpfg,
		mtVV,
		mtVVE,
		mtOther
	};

	Specifications();
	void Default();
	bool IsLoaded() const
	{ return bLoaded; }
	bool AutoRun() const
	{ return _IsFlagSet(flAutoRun); }
	void AutoRun(bool f)
	{ _FlagReset(flAutoRun, f); }
	eModelType ModelType() const
	{ return _modelType; }
	bool DemoMode() const
	{ return _IsFlagSet(flDemoMode); }
	void DemoMode(bool f)
	{ _FlagReset(flDemoMode, f); }
	bool ContinuousMode() const
	{ return _IsFlagSet(flContinuous); }
	void PanelsAutoTearOff(bool f)
	{ _FlagReset(flPanelsAutoTearOff, f); }
	bool PanelsAutoTearOff() const
	{ return _IsFlagSet(flPanelsAutoTearOff); }
	void AutoExit(bool f)
	{ _FlagReset(flAutoExit, f); }
	bool AutoExit() const
	{ return _IsFlagSet(flAutoExit); }
	bool ActiveTabSpecified() const
	{ return _mode != mUnspecified; }
	eProjectMode ActiveTab() const
	{
		assert(ActiveTabSpecified());
		return _mode; 
	}
	bool HasCmndLine() const
	{ return !_cmndln.empty(); }
	bool Ignored(const char* fname) const
	{ return _ignored.contains(fname); }
	const char* CmndLine() const
	{ return _cmndln.c_str(); }
	bool HasBuildLine() const
	{ return !_buildln.empty(); }
	const char* BuildLine() const
	{ return _buildln.c_str(); }
	void ReadNew(const std::string&, const std::string&);
	void ReadOld(const std::string&, const std::string&);
	const string_buffer& Ignored() const
	{ return _ignored; }
	const string_buffer& AlwaysSave() const
	{ return _alwaysSave; }
	const std::string& ModelFile() const
	{ return _modelFile; }
	bool IsModelFileSpecified() const
	{ return _modelFile.length()>0; }
	const std::string& SimulatorName() const
	{ return _simulatorName; }
	bool IsSimulatorNameSpecified() const
	{ return _simulatorName.length()>0; }
	size_t GetSimulatorCommandsCount() const
	{ return _simulatorCommands.size(); }
	const std::string& GetSimulatorCommandAction(size_t commandId) const;
	bool WriteBeforeCommand(size_t commandId) const;

	const std::vector<MenuManipulator::tMenuBuildData>& SimulatorCommands() const
	{ return _simulatorMenuCommands; }

	static size_t SimulatorCommandBaseId();
	static const char* kContinuousModelingLabel;
	static const UINT kContinuousModelingCommandId;

private:
	bool ReadCmndLine(const char*, const char*);
	bool ReadBuildLine(const char*);
	void ReadIgnored(std::ifstream&);
	void ReadAlwaysSave(std::ifstream& source);
	void ReadFileList(std::ifstream& source, string_buffer& fileList);
	void ReadSimulator(const char* line);
	void ReadSimulatorName(const char* line);
	void ReadSimulatorCommand(const char* line);
	void ReadModelFile(const char* line);
	void ReadActiveTab(const char*);
	bool FixCmndLine(const char*);

	enum eLabels
	{
		lIgnore = 0,
		lContinuous,
		lContModeling,
		lActiveTab,
		lAutoRun,
		lDemoMode,
		lPanelsAutoTearOff,
		lAutoExit,
		lBuildCmd,
		lAlwaysSave,
		lSimulator,
		lModelFile,
		lSimulatorName,
		lSimulatorCommand,

		elCount
	};
	enum eFlags
	{
		flAutoRun           = 1 << 0,
		flDemoMode          = 1 << 1,
		flContinuous        = 1 << 2,
		flPanelsAutoTearOff = 1 << 3,
		flAutoExit          = 1 << 4
	};
	bool bLoaded;
	std::string _cmndln, _buildln;
	string_buffer _ignored;
	string_buffer _alwaysSave;
	eProjectMode _mode;
	eModelType _modelType;
	std::string _modelFile;
	std::string _simulatorName;
	std::vector<MenuManipulator::tMenuBuildData> _simulatorMenuCommands;

	class SimulatorCommand
	{
	public:
		SimulatorCommand(const std::string& command, bool bWriteToLabTable) : 
		  mCommand(command), mbWriteToLabTable(bWriteToLabTable)
		{}
		const std::string& Command() const { return mCommand; }
		bool WriteToLabTable() const { return mbWriteToLabTable; }

	private:
		std::string mCommand;
		bool mbWriteToLabTable;
	};

	void AddSimulatorCommand(const std::string& label, bool bWrite, const char* line);
	void AddSeparatorCommand();
	void AddContinuousModeCommand();

	std::vector<SimulatorCommand> _simulatorCommands;
	static const char* _strLabels[];
};

#endif
