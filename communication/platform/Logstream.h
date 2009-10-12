#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include <ostream>

/**
 * Class for formatted output to a std::streambuf. Implements the << operator
 * which makes it identical in use with every std::ostream.
 *
 * Prints the current date and time and its name on the beginning of every line
 * of output.
 */
class Logstream {
// data members
private:
	std::ostream os;
	std::string name;
	bool newline;
// methods
public:
	Logstream(std::string n, std::streambuf * sb);
	std::streambuf* rdbuf(std::streambuf* sb);
	std::streambuf* rdbuf() const;
	
	template <typename T>
	friend Logstream& operator<<(Logstream& log, T const & t) {
		if(log.newline) {
			log.print_date();
			log.os << log.name << ": ";
			log.newline = false;	
		}
		log.os << t;
		return log;
	};

	friend Logstream& operator<<(Logstream& log, std::ostream& (*fn)(std::ostream&));
	
private:
	void print_date();

};


#endif
