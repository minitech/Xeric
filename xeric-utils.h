#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "string-buffer.h"

/* Some message output methods. */
#ifdef _WIN32
	#define warn puts
	#define success puts
#else
	void warn(const char * message) {
		printf("\033[22;33m%s\n\033[22;0m", message);
	}

	void success(const char * message) {
		printf("\033[22;32m%s\n\033[22;0m", message);
	}
#endif

/* Represents an IPv4 address.                                  *
 * We use this to convert the `unsigned long` from sockaddr_in. */
typedef union {
	unsigned long address;

	struct {
		unsigned char p1;
		unsigned char p2;
		unsigned char p3;
		unsigned char p4;
	} components;
} ip4_t;

/* Handles interrupts gracefully.                            *
 * Oddly, however, interrupting currently causes a segfault. * <-- Actually, it doesn't anymore... uh-oh.
 * I have no idea where that comes from.                     */
int keepRunning = 1;
int s;

void interrupted(int sig) {
	/* Tell it to stop running: */
	keepRunning = 0;

	/* Close the socket: */
	close(s);
	s = -1;

	/* Make it possible to "really kill": */
	signal(SIGINT, SIG_DFL);
}

/* Parses a port number, since strtoll isn't available in C89. */
int strtoport(const char * str) {
	int p = 0;
	int v;

	while(*str) {
		v = *str++ - '0';

		if(v < 0 || v > 9)
			return 0;

		p = p * 10 + v;

		if(p > 65535)
			return 0;
	}

	return p;
}