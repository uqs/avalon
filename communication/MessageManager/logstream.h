#ifndef LOGSTREAM_H
#define LOGSTREAM_H

#include <ostream>


class logstream {
	std::ostream os;
	bool date;
	public:
		logstream(std::streambuf * sb) : os(sb), date(true) {};
		std::streambuf* rdbuf(std::streambuf* sb) { return os.rdbuf(sb); };
		std::streambuf* rdbuf() const { return os.rdbuf();};
		void print_date();
		
		friend logstream& operator<<(logstream& log, std::ostream& (*fn)(std::ostream&));
		
		template <typename T>
		friend logstream& operator<<(logstream& log, T const & t) {
			if(log.date) {
				log.print_date();
				log.date = false;	
			}
			log.os << t;
			return log;
		};
};


#endif
