/*
 * main.c
 *
 *  Created on: Dec 5, 2014
 *      Author: baoke
 */

#include <signal.h>
#include "h3c.h"

//#define DEBUG

void success_handler()
{
	printf("Succeed to go on line\n");
	daemon(0, 0);
}

void failure_handler()
{
	printf("Failed to go on line\n");
}

void exit_handler(int arg)
{
	h3c_logoff();
	h3c_clean();
	exit(0);
}

int main(int argc, char **argv)
{
	/*
	 * Check privilege
	 */
	if (geteuid() != 0)
	{
		printf("Run as root\n");
		exit(-1);
	}

	/*
	 * Check arguments
	 */
	if (argc != 4)
	{
		printf("Usage:%s [interface] [username] [password]\n", argv[0]);
		exit(-1);
	}

	h3c_set_verbose(1);

	if (h3c_init(argv[1]) == -1)
	{
		printf("Failed to initialize: %s\n",strerror(errno));
		exit(-1);
	}

	//signal(SIGTERM, exit_handler);

	h3c_set_username(argv[2]);
	h3c_set_password(argv[3]);

	if (h3c_start() == -1)
	{
		printf("Failed to start: %s\n", strerror(errno));
		exit(-1);
	}

	for(;;)
	{
		if (h3c_response(success_handler, failure_handler) == -1)
		{
			printf("Failed to response: %s\n", strerror(errno));
			exit(-1);
		}
	}

	return 0;
}
