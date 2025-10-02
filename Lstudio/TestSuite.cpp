#include <fw.h>

#include "TestSuite.h"

#include "specifications.h"
#include "lstudioptns.h"

#include <browser/connection.h>
#include <browser/localcnct.h>
#include <browser/remotecnct.h>

#define Verify(command)																\
	{																				\
		bool bLocal = (command);													\
		if (!bLocal)																\
		{																			\
			testLog << "Test error ["												\
				<< #command << "]"													\
				<< " at " << __FILE__ << ", " << __LINE__ << std::endl;				\
		}																			\
		bRes = bRes && bLocal;														\
	}


bool TestSuite::Testspecifications(std::ostream& testLog)
{
	bool bRes = true;

	Specifications specifications;
	specifications.ReadOld("specifications.empty", "color.map");

	Verify(1);

	return bRes;
}


bool TestSuite::TestSpecifications(std::ostream& testLog)
{
	bool bRes = true;

	Specifications specifications;
	specifications.ReadNew("LSspecifications", "");

	Verify(specifications.AlwaysSave().contains("lsystem.str"));
	Verify(specifications.Ignored("lsys.dll"));
	Verify(specifications.IsModelFileSpecified());
	Verify(0==specifications.ModelFile().compare("lsystem.l"));
	Verify(specifications.IsSimulatorNameSpecified());
	Verify(0==specifications.SimulatorName().compare("Super Cool Simulator"));
	Verify(specifications.GetSimulatorCommandsCount() == 3);
	Verify(0==specifications.GetSimulatorCommandAction(0).compare("run.bat"));
	Verify(0==specifications.GetSimulatorCommandAction(1).compare("shoot.bat"));
	Verify(0==specifications.GetSimulatorCommandAction(2).compare("run.bat -again"));

	return bRes;
}


bool TestSuite::TestSpecificationsA(std::ostream& testLog)
{
	bool bRes = true;

	Specifications specifications;
	specifications.ReadNew("LSspecifications.cpfg", "");
	Verify(specifications.ModelType() == Specifications::mtCpfg);
	Verify(!specifications.IsSimulatorNameSpecified());
	Verify(specifications.GetSimulatorCommandsCount() == 0);

	return bRes;
}



bool TestSuite::TestSpecificationsB(std::ostream& testLog)
{
	bool bRes = true;

	Specifications specifications;
	specifications.ReadNew("LSspecifications.lpfg", "");
	Verify(specifications.ModelType() == Specifications::mtLpfg);
	Verify(!specifications.IsSimulatorNameSpecified());
	Verify(specifications.GetSimulatorCommandsCount() == 0);

	return bRes;
}



bool TestSuite::TestSpecificationsC(std::ostream& testLog)
{
	bool bRes = true;

	Specifications specifications;
	specifications.ReadNew("LSspecifications.vv", "");
	Verify(specifications.ModelType() == Specifications::mtVV);
	Verify(!specifications.IsSimulatorNameSpecified());
	Verify(specifications.GetSimulatorCommandsCount() == 0);

	return bRes;
}



bool TestSuite::TestSpecificationsD(std::ostream& testLog)
{
	bool bRes = true;

	Specifications specifications;
	specifications.ReadNew("LSspecifications.custom", "");
	Verify(specifications.ModelType() == Specifications::mtOther);
	Verify(!specifications.IsSimulatorNameSpecified());
	Verify(specifications.GetSimulatorCommandsCount() == 4);
	Verify(0==specifications.SimulatorCommands()[2].szLabel.compare("This will not write to lab table"));
	Verify(0==specifications.GetSimulatorCommandAction(2).compare("DoOtherStuff.bat"));
	Verify(0==specifications.GetSimulatorCommandAction(3).compare("DoMoreStuff.bat"));
	Verify(specifications.WriteBeforeCommand(0));
	Verify(specifications.WriteBeforeCommand(1));
	Verify(!specifications.WriteBeforeCommand(2));
	Verify(specifications.WriteBeforeCommand(3));

	return bRes;
}



bool TestSuite::TestLStudioOptions(std::ostream& testLog)
{
	bool bRes = true;

	LStudioOptions options;
	ReadTextFile source("options.cfg");
	options.Load(source);

	Verify(options.GetHelpCommandCount() == 4);
	LStudioOptions::HelpCommand helpCommand;
	helpCommand = options.GetHelpCommand(0);
	Verify(helpCommand.mLabel.compare("Help1") == 0);
	Verify(helpCommand.mCommand.compare("doHelp1.bat") == 0);
	helpCommand = options.GetHelpCommand(1);
	Verify(helpCommand.mLabel.compare("Help2") == 0);
	Verify(helpCommand.mCommand.compare("doHelp2.bat") == 0);
	helpCommand = options.GetHelpCommand(2);
	Verify(helpCommand.mLabel.compare("") == 0);
	Verify(helpCommand.mCommand.compare("\"\"\"") == 0);
	helpCommand = options.GetHelpCommand(3);
	Verify(helpCommand.mLabel.compare("Command with parameters") == 0);
	Verify(helpCommand.mCommand.compare("command.com param1 param2 \"c:\\temp with space\\log.txt\"") == 0);

	return bRes;
}


bool TestSuite::TestConfigFile(std::ostream& testLog)
{
	bool bRes = true;

	const char* testString = "string \"That will be\" tested. Here!";

	class TestConfig : public ConfigFile
	{
	public:
		TestConfig(const char* str) : _str(str), _origString(_str)
		{}

		bool Test1()
		{
			_str = FindNextBlank(_str);
			return (_str == _origString + 6);
		}

		bool Test2()
		{
			_str = SkipBlanks(_str);
			std::string quote;
			_str = ReadQuotedString(_str, quote);
			return (0==quote.compare("That will be") && _str == _origString + 21);
		}

		bool Test3()
		{
			_str = SkipBlanks(_str);
			return (_str == _origString + 22);
		}

		const char* _str;
		const char* _origString;
	};

	TestConfig config(testString);
	Verify(config.Test1());
	Verify(config.Test2());
	Verify(config.Test3());

	return bRes;
}



bool TestSuite::TestLocalConnection(std::ostream& /*testLog*/)
{
	bool bRes = true;


	return bRes;
}



bool TestSuite::TestRAConnection(std::ostream& testLog)
{
	bool bRes = true;

	VLB::Connection* pConnection = NULL;

	string_buffer crlf;
	const char* initData[] = 
	{
		"l", "v", "a", 
		"fset", "cset", 
		"s", "e", 
		"LSspecifications", 
		"specifications", 
		"txt", NULL
	};

	for (int ix = 0; ; ++ix)
	{
		const char* lbl = initData[ix];
		if (NULL == lbl)
			break;
		crlf.add_str(lbl);
	}

	try
	{
		pConnection = new VLB::RemoteConnection("vlabx.cpsc.ucalgary.ca", "radekk", "radekk", crlf);
	}
	catch (Exception e)
	{
		testLog << "Error creating RA connection: " << e.Msg() << std::endl;
	}
	Verify(pConnection != NULL);
	Verify(BasicConnectionTests(pConnection, testLog));

	delete pConnection;
	return bRes;
}


bool TestSuite::BasicConnectionTests(VLB::Connection* /*pConnection*/, std::ostream& /*logFile*/)
{
	bool bRes = true;




	return bRes;
}
