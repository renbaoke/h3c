/*
 * main.c
 * 
 * Copyright 2015 BK <renbaoke@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

/*
 *  Created on: Dec 5, 2014
 *      Author: BK <renbaoke@gmail.com>
 */

#include <signal.h>
#include "h3c.h"

void success_handler()
{
	printf("You are now online.\n");
	daemon(0, 0);
}

void failure_handler()
{
	printf("You are now offline.\n");
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
	printf("  -i <interface>\tspecify interface, required\n");
	printf("  -u <username>\t\tspecify username, required\n");
	printf("  -p <password>\t\tspecify password, optional\n");
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
