#ifndef LOG_H_
#define LOG_H_

#include "Logstream.h"


class Log
{
public:
	enum LogLevel {
		Normal,
		Error
	};
	
protected:
	Logstream info;
	Logstream error;


public:
	Log(std::string name);
	void set_logfile(std::streambuf *logfile);
};

#endif /*LOG_H_*/
