#ifndef IREPORT_LOGGER_H
#define IREPORT_LOGGER_H

#include <iostream>

class IReportLog
{
public:
	virtual std::ostream& failed() =0;
	virtual std::ostream& succeeded() =0;
	~IReportLog(){}
};

class IReportLogger
{
public:
	virtual void run(IReportLog&) =0;
	~IReportLogger(){}
};

#endif

