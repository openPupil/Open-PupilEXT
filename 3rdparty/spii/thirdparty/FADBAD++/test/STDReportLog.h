#ifndef STD_REPORT_LOG_H
#define STD_REPORT_LOG_H

#include <iostream>
#include "IReportLogger.h"

class STDReportLog : public IReportLog
{
	std::ostream& m_os;
	long m_failed;
	long m_succeeded;
	void operator=(const STDReportLog&) {/*not allowed*/}
public:
	STDReportLog(std::ostream& os):m_os(os),m_failed(0),m_succeeded(0){}
	~STDReportLog()
	{
		m_os << "Total failed :" << m_failed << std::endl;
		m_os << "Total succeeded :" << m_succeeded << std::endl;
		m_os.flush();
	}
	std::ostream& failed(){m_failed++;m_os<<"FAILED: ";return m_os;}
	std::ostream& succeeded(){m_succeeded++;m_os<<"SUCCEEDED: ";return m_os;}
};

#endif

