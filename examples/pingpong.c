// Ping-pong a counter between two processes.
// Only need to start one of these -- splits into two with fork.

#include <stdio.h>

#include <vireo.h>

void pingpong(void);

void
umain(void)
{
	int who = vireo_create(pingpong);

	// get the ball rolling
	printf("send 0 from %x to %x\n", vireo_getid(), who);
	vireo_send(who, 0);

	pingpong();
}

void
pingpong(void)
{
	int who;
	while (1) {
		int i = vireo_recv(&who);
		printf("%x got %d from %x\n", vireo_getid(), i, who);
		if (i == 10)
			return;
		i++;
		vireo_send(who, i);
		if (i == 10)
			return;
	}
}
