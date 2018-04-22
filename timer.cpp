#ifndef TIMER
#define TIMER

#include <sys/types.h>
#include <sys/time.h>
#include <ctime>

typedef struct Timer {
	struct timeval time;
	time_t GetTimeInSeconds() {
		gettimeofday(&time, NULL);
		return time.tv_sec;
	}
	
	suseconds_t GetTimeInMicroSeconds() {
		gettimeofday(&time, NULL);
		return time.tv_usec;
	}
} Timer;

#endif