#include <fstream>

#include <fw.h>

#include "specifications.h"
#include "resource.h"

const size_t kBaseSimulatorCommandId = 46000;
const char* kContinuousModeling = "Continuous modeling";
const char* Specifications::kContinuousModelingLabel = "C&ontinuous modeling";
const UINT Specifications::kContinuousModelingCommandId = ID_SIMULATION_CONTINUOUSMODELING;

const char* Specifications::_strLabels[] =
{
	"ignore:",
	"continuous mode:",
	"continuous modeling:",
	"active tab:",
	"auto run:",
	"demo mode:",
	"panels auto tear off:",
	"auto exit:",
	"build command:",
	"always save:",
	"simulator:",
	"model file:",
	"simulator name:",
	"simulator command:"
};




Specifications::Specifications()
{
	Default();
}


void Specifications::Default()
{
	bLoaded = false;
	AutoRun(false);
	DemoMode(false);
	PanelsAutoTearOff(false);
	AutoExit(false);
	_cmndln.clear();
	_buildln.clear();
	_ignored.reset();
	_alwaysSave.reset();
	_modelFile.clear();
	_simulatorName.clear();
	_simulatorMenuCommands.clear();
	_simulatorCommands.clear();
	_mode = mUnspecified;
	_modelType = mtCpfg;
}


void Specifications::ReadNew(const std::string& fname, const std::string& mapfile)
{
	Default();
	std::ifstream src(fname);
	if (!src.is_open())
		return;

	std::string line;

	while (!src.eof())
	{
		std::getline(src, line);
		int cntn = -1;
		int lbl = _Label(line.c_str(), cntn, _strLabels, elCount);
		switch (lbl)
		{
		case -1 :
			ReadCmndLine(line.c_str(), mapfile.c_str());
			break;
		case lBuildCmd:
			ReadBuildLine(line.c_str()+cntn);
			break;
		case lIgnore:
			ReadIgnored(src);
			break;
		case lContinuous:
		case lContModeling :
			_ReadOnOff(line.c_str()+cntn, flContinuous);
			break;
		case lActiveTab :
			ReadActiveTab(line.c_str()+cntn);
			break;
		case lAutoRun:
			_ReadOnOff(line.c_str()+cntn, flAutoRun);
			break;
		case lDemoMode :
			_ReadOnOff(line.c_str()+cntn, flDemoMode);
			break;
		case lPanelsAutoTearOff :
			_ReadOnOff(line.c_str()+cntn, flPanelsAutoTearOff);
			break;
		case lAutoExit :
			_ReadOnOff(line.c_str()+cntn, flAutoExit);
			break;
		case lAlwaysSave:
			ReadAlwaysSave(src);
			break;
		case lSimulator:
			ReadSimulator(line.c_str() + cntn);
			break;
		case lModelFile:
			ReadModelFile(line.c_str() + cntn);
			break;
		case lSimulatorName:
			ReadSimulatorName(line.c_str() + cntn);
			break;
		case lSimulatorCommand:
			ReadSimulatorCommand(line.c_str()+cntn);
			break;
		}
	}

	bLoaded = true;

}

void Specifications::ReadActiveTab(const char* cmnd)
{
	struct ActiveTabPair
	{
		const char* lbl;
		eProjectMode mode;
	};
	const ActiveTabPair tabs[] =
	{
		{ "lsystem",     mLsystem },
		{ "l-system",    mLsystem },
		{ "view",        mView },
		{ "anim",        mAnimate },
		{ "animate",     mAnimate },
		{ "color",       mColors },
		{ "colors",      mColors },
		{ "surface",     mSurface },
		{ "surfaces",    mSurface },
		{ "contour",     mContours },
		{ "contours",    mContours },
		{ "function",    mFunctions },
		{ "functions",   mFunctions },
		{ "panel",       mPanels },
		{ "panels",      mPanels },
		{ "description", mDescription },
		{ "anytext",     mAnyText },
		{ "text file",   mAnyText },
		{ 0,             mUnspecified }
	};

	cmnd = SkipBlanks(cmnd);
	_mode = mUnspecified;
	for (int i=0; tabs[i].lbl != 0; ++i)
	{
		if (0==_stricmp(cmnd, tabs[i].lbl))
		{
			_mode = tabs[i].mode;
			break;
		}
	}
}


void Specifications::ReadAlwaysSave(std::ifstream& source)
{
	ReadFileList(source, _alwaysSave);
}

void Specifications::ReadIgnored(std::ifstream& src)
{
	ReadFileList(src, _ignored);
}

void Specifications::ReadFileList(std::ifstream& source, string_buffer& fileList)
{
	std::string line;
	while (!source.eof())
	{
		std::getline(source, line);
		if ('*' != line[0])
			fileList.add(line.c_str());
		else
			break;
	}
}

bool Specifications::ReadCmndLine(const char* line, const char* mapfile)
{
	line = SkipBlanks(line);
	if (0==strncmp(line, "cpfg", 4))
	{
		_modelType = mtCpfg;
		_cmndln = line;
		if (FixCmndLine(mapfile))
			return true;
		else
		{
			_cmndln.erase();
			return false;
		}
	}
	else if (0==strncmp(line, "lpfg", 4))
	{
		_modelType = mtLpfg;
		_cmndln = line;
		return true;
	}
	else if (0==strncmp(line, "vve", 3))
	{
		_modelType = mtVVE;
		_cmndln = line;
		return true;
	}
	else if (0==strncmp(line, "vv", 2))
	{
		_modelType = mtVV;
		_cmndln = line;
		return true;
	}
	else if (_cmndln.empty())
	{
		_modelType = mtOther;
		_cmndln = line;
	}
	return false;
}


void Specifications::ReadSimulator(const char* line)
{
	_modelType = mtOther;
	line = SkipBlanks(line);
	_cmndln = line;
}

void Specifications::ReadModelFile(const char* line)
{
	line = SkipBlanks(line);
	_modelFile = line;
}

void Specifications::ReadSimulatorName(const char* line)
{
	line = SkipBlanks(line);
	_simulatorName = line;
}

bool Specifications::ReadBuildLine(const char* line)
{
	line = SkipBlanks(line);
	if (_buildln.empty())
		_buildln = line;
	return false;
}


void Specifications::ReadOld(const std::string& fname, const std::string& mapfile)
{
	Default();
	std::ifstream src(fname);
	if (!src.is_open())
		return;

	std::string line;
	while (!src.eof())
	{
		std::getline(src, line);
		if (line.length()>0 && line[0] == '\t')
		{
			if (ReadCmndLine(line.c_str(), mapfile.c_str()))
				break;
		}
	}

	bLoaded = true;
}


bool Specifications::FixCmndLine(const char* mapfile)
{

	std::string::iterator it(_cmndln.begin());
	it += 4;
	while (!isspace(*it) && (_cmndln.length()>4))
		_cmndln.erase(it);
	if (_cmndln.length()==4)
		return false;
	size_t pos = _cmndln.find("-M");
	if (std::string::npos != pos)
		return true;
	pos = _cmndln.find("-m");
	if (std::string::npos != pos)
		return true;
	std::string ins("-m ");
	ins.append(mapfile);
	ins.append(" ");
	_cmndln.insert(5, ins);
	return true;
}


void Specifications::ReadSimulatorCommand(const char* line)
{
	_modelType = mtOther;


	std::string label;
	// find \"
	line = SkipBlanks(line);
	if (NULL == line || *line != '\"')
		return;

	line = ReadQuotedString(line, label);
	if (NULL == line)
		return;

	if (0==label.compare(MenuManipulator::tMenuBuildData::kSeparatorTag))
	{
		AddSeparatorCommand();
		return;
	}
	else if (0==label.compare(kContinuousModeling))
	{
		AddContinuousModeCommand();
		return;
	}

	line = SkipBlanks(line);
	if (line == NULL || *line == 0)
		return;

	
	bool bWrite = true;
	const char* nextBlank = FindNextBlank(line);

	const char* kWriteOn = "+w";
	const char* kWriteOff = "-w";

	if (0==strncmp(kWriteOn, line, nextBlank-line))
	{
		bWrite = true;
		line = SkipBlanks(nextBlank);
	}
	else if (0==strncmp(kWriteOff, line, nextBlank-line))
	{
		bWrite = false;
		line = SkipBlanks(nextBlank);
	}

	AddSimulatorCommand(label, bWrite, line);
}


void Specifications::AddSeparatorCommand()
{
	MenuManipulator::tMenuBuildData simulatorCommand;
	simulatorCommand.szLabel = MenuManipulator::tMenuBuildData::kSeparatorTag;
	simulatorCommand.uiCommandId = SimulatorCommandBaseId() + _simulatorMenuCommands.size();
	_simulatorMenuCommands.push_back(simulatorCommand);

	SimulatorCommand Command("", false);
	_simulatorCommands.push_back(Command);
}



void Specifications::AddSimulatorCommand(const std::string& label, bool bWrite, const char* line)
{
	std::string command(line);

	MenuManipulator::tMenuBuildData simulatorCommand;
	simulatorCommand.szLabel = label;
	simulatorCommand.uiCommandId = SimulatorCommandBaseId() + _simulatorMenuCommands.size();
	_simulatorMenuCommands.push_back(simulatorCommand);

	SimulatorCommand Command(command, bWrite);
	_simulatorCommands.push_back(Command);
}


void Specifications::AddContinuousModeCommand()
{
	MenuManipulator::tMenuBuildData simulatorCommand;
	simulatorCommand.szLabel = Specifications::kContinuousModelingLabel;
	simulatorCommand.uiCommandId = Specifications::kContinuousModelingCommandId;
	_simulatorMenuCommands.push_back(simulatorCommand);

	SimulatorCommand Command("", false);
	_simulatorCommands.push_back(Command);
}

size_t Specifications::SimulatorCommandBaseId()
{
	return kBaseSimulatorCommandId;
}


const std::string& Specifications::GetSimulatorCommandAction(size_t commandId) const
{
	assert(commandId < _simulatorCommands.size());
	return _simulatorCommands[commandId].Command();
}


bool Specifications::WriteBeforeCommand(size_t commandId) const
{
	assert(commandId < _simulatorCommands.size());
	return _simulatorCommands[commandId].WriteToLabTable();
}

