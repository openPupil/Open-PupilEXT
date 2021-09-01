#ifndef TEST_UDT_H
#define TEST_UDT_H

#include "IReportLogger.h"

class TestUDT : public IReportLogger
{
	public:
	void run(IReportLog& rlog);
};

#endif


