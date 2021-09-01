#ifndef TEST_TAD_H
#define TEST_TAD_H

#include "IReportLogger.h"

class TestTAD : public IReportLogger
{
	public:
	void run(IReportLog& rlog);
};

#endif
