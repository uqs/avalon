#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <DDXStore.h>
#include <DDXVariable.h>

DDX_STORE_TYPE(SystemState,
		struct {
			float uptime;
			unsigned int nuser;
			float loadavg1;
			float loadavg5;
			float loadavg15;
			unsigned int totalram;
			unsigned int usedram;
			unsigned int freeram;
			float cputemp;
			float gputemp;
			float fanrpm;
		}
);



#endif // SYSTEM_STATE_H
