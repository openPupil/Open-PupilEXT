#include "STDReportLog.h"

#include "TestFADBAD.h"
#include "TestTAD.h"
#include "TestUDT.h"

int main()
{
	STDReportLog log(std::cerr);

	TestFADBAD testFADBAD;
	testFADBAD.run(log);

	TestTAD testTAD;
	testTAD.run(log);

	TestUDT testUDT;
	testUDT.run(log);

	return 0;
}
