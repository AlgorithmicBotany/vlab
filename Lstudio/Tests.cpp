
#include <fw.h>

#include "TestSuite.h"


int RunTests()
{
	bool bRes = true;
	std::ofstream testLog("Lstudio-test.log");

	TestSuite testSuite;
	bRes = testSuite.Testspecifications(testLog) && bRes;
	bRes = testSuite.TestSpecifications(testLog) && bRes;
	bRes = testSuite.TestSpecificationsA(testLog) && bRes;
	bRes = testSuite.TestSpecificationsB(testLog) && bRes;
	bRes = testSuite.TestSpecificationsC(testLog) && bRes;
	bRes = testSuite.TestSpecificationsD(testLog) && bRes;
	bRes = testSuite.TestLStudioOptions(testLog) && bRes;
	bRes = testSuite.TestConfigFile(testLog) && bRes;
	bRes = testSuite.TestLocalConnection(testLog) && bRes;
	bRes = testSuite.TestRAConnection(testLog) && bRes;

	if (bRes)
	{
		testLog << "All tests passed\n";
	}
	return bRes;
}

