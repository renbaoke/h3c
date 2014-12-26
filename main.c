/*
 * main.c
 *
 *  Created on: Dec 5, 2014
 *      Author: baoke
 */

#include "h3c.h"

int success_handler()
{
	printf("Succeed to go on line\n");
	printf("Try to run \"dhclient [interface]\" as root\n");
	daemon(0, 0);
	return 0;
}

int failure_hander()
{
	printf("Failed\n");
	return 0;
}

int main(int argc, char **argv)
{
	/*
	 * Check privilege
	 */
	if (0 != geteuid())
	{
		printf("Run as root\n");
		exit(-1);
	}

	/*
	 * Check arguments
	 */
	if (4 != argc)
	{
		printf("Usage:%s [interface] [username] [password]\n", argv[0]);
		exit(-1);
	}

	if (-1 == h3c_init(argv[1]))
	{
		printf("Failed to initialize: %s\n",strerror(errno));
		exit(-1);
	}

	h3c_set_username(argv[2]);
	h3c_set_password(argv[3]);

	if (-1 == h3c_start())
	{
		printf("Failed to start: %s\n", strerror(errno));
		exit(-1);
	}

	for(;;)
	{
		h3c_response(success_handler, failure_hander);
	}

	return 0;
}
