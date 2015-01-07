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
	printf("You are now online\n");
	daemon(0, 0);
}

void failure_handler()
{
	printf("You are now offline\n");
}

void exit_handler(int arg)
{
	printf("\nExiting...\n");
	h3c_logoff();
	h3c_clean();
	exit(0);
}

void verbose_handler(char *msg)
{
	printf("%s", msg);
}

void usage()
{
	printf("Usage: h3c [OPTION]...\n");
	printf("  -i <interface>\tset interface, required\n");
	printf("  -u <username>\t\tset username, required\n");
	printf("  -p <password>\t\tset password, optional\n");
	printf("  -v\t\t\tverbose, optional\n");
	printf("  -h\t\t\tshow this message\n");
}

int main(int argc, char **argv)
{
	int ch;
	int vflag = 0;
	char *interface = NULL;
	char *username = NULL;
	char *password = NULL;

	while ((ch = getopt(argc, argv, "i:u:p:vh")) != -1)
	{
		switch(ch)
		{
			case 'h':
				usage();
				exit(0);
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

	//Must run as root.
	if (geteuid() != 0)
	{
		printf("Run as root\n");
		exit(-1);
	}

	if (interface == NULL || username == NULL)
	{
		usage();
		exit(-1);
	}

	if (password == NULL)
	{
		printf("Password for %s:", username);
		password = getpass("");
	}

	if (vflag)
		h3c_set_verbose(verbose_handler);

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
