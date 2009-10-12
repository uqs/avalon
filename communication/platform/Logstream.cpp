
#include "Logstream.h"

#include <iomanip>

Logstream::Logstream(std::string n, std::streambuf * sb) : os(sb), name(n), newline(true)
{
}

/**
 * Set the underlying streambuf object.
 *
 * @param	sb
 * @return	the previous streambuf object
 */
std::streambuf* Logstream::rdbuf(std::streambuf* sb)
{
	 return os.rdbuf(sb);
}

/**
 * Get the underlying streambuf object.
 *
 * @return
 */
std::streambuf* Logstream::rdbuf() const
{
	 return os.rdbuf();
}

/**
 * Print the date to the output stream in the format 2009-07-08 08:25:27
 */
void Logstream::print_date()
{
	time_t at;
	struct tm rt;
	time(&at);
	gmtime_r(&at, &rt);
	os << std::setfill('0');
	os << std::setw(2) << rt.tm_year+1900 << '-'
	   << std::setw(2) << rt.tm_mon+1 << '-'
	   << std::setw(2) << rt.tm_mday << ' '
	   << std::setw(2) << rt.tm_hour << ':'
	   << std::setw(2) << rt.tm_min << ':'
	   << std::setw(2) << rt.tm_sec << ' ';
}

Logstream& operator<<(Logstream& log, std::ostream& (*fn)(std::ostream&))
{
	if(fn == static_cast<std::ostream& (*)(std::ostream&)>(std::endl<char, std::char_traits<char> >)) {
		log.newline = true;
	}
	fn(log.os);
	return log;
}
