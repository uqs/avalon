
#include "logstream.h"

#include <iomanip>

using namespace std;

void logstream::print_date()
{
	time_t at;
	struct tm rt;
	time(&at);
	gmtime_r(&at, &rt);
	os << std::setfill('0') << std::setw(2);
	os << rt.tm_year+1900 << '-' << setw(2) << rt.tm_mon+1 << '-' << setw(2) << rt.tm_mday << ' ' << setw(2)
	   << rt.tm_hour << ':' << setw(2) << rt.tm_min << ':' << setw(2) << rt.tm_sec << ' ';
}

logstream& operator<<(logstream& log, std::ostream& (*fn)(std::ostream&))
{
	if(fn == static_cast<std::ostream& (*)(std::ostream&)>(std::endl<char, std::char_traits<char> >)) {
		log.date = true;
	}
    fn(log.os);
    return log;
}