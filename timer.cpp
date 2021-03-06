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

	suseconds_t GetCurrentTimeInMicroSeconds() {
		gettimeofday(&time, NULL);
		return time.tv_sec * 1000000 + time.tv_usec;
	}

	int MicroSecToSec(int &microseconds) {
		int seconds = 0;
		while (microseconds >= 1000000) {
			microseconds -= 1000000;
			seconds++;
		}

		return seconds;
	}
} Timer;

#endif