#ifndef __TESTSUITE_H__
#define __TESTSUITE_H__

#include <fstream>


namespace VLB
{
	class Connection;
}

class TestSuite
{
public:

	bool Testspecifications(std::ostream& logFile);
	bool TestSpecifications(std::ostream& logFile);
	bool TestSpecificationsA(std::ostream& logFile);
	bool TestSpecificationsB(std::ostream& logFile);
	bool TestSpecificationsC(std::ostream& logFile);
	bool TestSpecificationsD(std::ostream& logFile);
	bool TestLStudioOptions(std::ostream& logFile);
	bool TestConfigFile(std::ostream& logFile);
	bool TestLocalConnection(std::ostream& logFile);
	bool TestRAConnection(std::ostream& logFile);


private:

	bool BasicConnectionTests(VLB::Connection* pConnection, std::ostream& logFile);
};


#endif
