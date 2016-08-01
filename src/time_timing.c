/* Feature test macro for nice(2). */
#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif /* _BSD_SOURCE */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>

int main(void)
{
	struct timeval tv_a = { 0 }, tv_b = { 0 };
	struct rlimit rlim = { 0 };
	int priority;

	/* Get as big a CPU timeslice as possible. */
	if (getrlimit(RLIMIT_NICE, &rlim) == -1) {
		perror("getrlimit RLIMIT_NICE");
		return EXIT_FAILURE;
	}

	errno = 0;
	priority = getpriority(PRIO_PROCESS, getpid());
	if (errno) {
		perror("getpriority on self process");
		return EXIT_FAILURE;
	}
	
	if (rlim.rlim_cur < rlim.rlim_max) {
		rlim.rlim_cur = rlim.rlim_max;
		if (!setrlimit(RLIMIT_NICE, &rlim)) {
			perror("setrlimit RLIMIT_NICE max");
			return EXIT_FAILURE;
		}
	}

	if (nice(-priority) == -1) {
		perror("nice to highest priority");
		return EXIT_FAILURE;
	}

	/**
	 * Time how long it takes to get the time, to be able to do
	 * time_functionality_real == (time_functionality - time_timing)
	 *
	 * time_functionality is the diffed time from:
	 *	gettimeofday
	 *	functionality
	 *	gettimeofday
	 *
	 * time_timing is the diffed time from:
	 *	gettimeofday
	 *	gettimeofday
	 */
	if (gettimeofday(&tv_a, NULL) == -1)
		goto exit;
	if (gettimeofday(&tv_b, NULL) == -1)
		goto exit;

	printf("%u\n", (unsigned int) (tv_b.tv_usec - tv_a.tv_usec));

exit: /* Fallthrough. */
	return EXIT_SUCCESS;
}

