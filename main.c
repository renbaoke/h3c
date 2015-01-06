/*
 * main.c
 *
 *  Created on: Dec 5, 2014
 *      Author: baoke
 */

#include <signal.h>
#include "h3c.h"

void success_handler()
{
	printf("Online now\n");
	daemon(0, 0);
}

void failure_handler()
{
	printf("Offline now\n");
}

void exit_handler(int arg)
{
	printf("\nExiting...\n");
	h3c_logoff();
	h3c_response(NULL, failure_handler);
	h3c_clean();
	exit(0);
}

void usage()
{
	printf("usage\n");
}

int main(int argc, char **argv)
{
	int ch;
	int vflag = 0;
	char *interface = NULL;
	char *username = NULL;
	char *password = NULL;

	//Must run as root.
	if (geteuid() != 0)
	{
		printf("Run as root\n");
		exit(-1);
	}

	while ((ch = getopt(argc, argv, "i:u:p:v")) != -1)
	{
		switch(ch)
		{
			case 'i':
				interface = optarg;
				break;
			case 'u':
				username = optarg;
				break;
			case 'p':
				password = optarg;
				break;
			case 'v':
				vflag = 1;
				break;
			default:
				usage();
				exit(-1);
		}
	}

	if (interface == NULL || username == NULL)
	{
		usage();
		exit(-1);
	}

	if (password == NULL)
		password = getpass("Password:");

	h3c_set_verbose(vflag);
	h3c_set_username(username);
	h3c_set_password(password);

	if (h3c_init(interface) == -1)
	{
		printf("Failed to initialize: %s\n",strerror(errno));
		exit(-1);
	}

	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

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
