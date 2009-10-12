#include "Log.h"

#include <iostream>
#include <fstream>

Log::Log(std::string name) : info(name, std::cout.rdbuf()), error(name, std::cerr.rdbuf())
{
}

void Log::set_logfile(std::streambuf *logfile)
{
	info.rdbuf(logfile);
	error.rdbuf(logfile);
}
