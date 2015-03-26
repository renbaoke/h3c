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

#include <signal.h>
#include "h3c.h"

int success_handler()
{
	printf("You are now ONLINE.\n");
	daemon(0, 0);
	return SUCCESS;
}

int failure_handler()
{
	printf("You are now OFFLINE.\n");
	return SUCCESS;
}

int unkown_eapol_handler()
{
	return SUCCESS;
}

int unkown_eap_handler()
{
	return SUCCESS;
}

/* We should NOT got response messages and we ignore them. */
int got_response_handler()
{
	return SUCCESS;
}

void exit_handler(int arg)
{
	printf("\nExiting...\n");
	h3c_logoff();
	h3c_clean();
	exit(0);
}

void usage()
{
	printf("Usage: h3c [OPTION]...\n");
	printf("  -i <interface>\tspecify interface, required\n");
	printf("  -u <username>\t\tspecify username, required\n");
	printf("  -p <password>\t\tspecify password, optional\n");
	printf("  -h\t\t\tshow this message\n");
}

int main(int argc, char **argv)
{
	int ch;
	char *interface = NULL;
	char *username = NULL;
	char *password = NULL;

	while ((ch = getopt(argc, argv, "i:u:p:h")) != -1)
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
			default:
				usage();
				exit(-1);
		}
	}

	/* Must run as root. */
	if (geteuid() != 0)
	{
		printf("Run as root.\n");
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


	if (h3c_set_username(username) != SUCCESS)
	{
		printf("Username too long!");
		exit(-1);
	}
	
	if (h3c_set_password(password) != SUCCESS)
	{
		printf("Password too long!");
		exit(-1);
	}

	if (h3c_init(interface) != SUCCESS)
	{
		printf("Failed to initialize: %s\n",strerror(errno));
		exit(-1);
	}

	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	if (h3c_start() != 0)
	{
		printf("Failed to start: %s\n", strerror(errno));
		exit(-1);
	}

	for(;;)
	{
		if (h3c_response(success_handler, failure_handler, \
				unkown_eapol_handler, unkown_eap_handler, \
				got_response_handler) != 0)
		{
			printf("Failed to response: %s\n", strerror(errno));
			exit(-1);
		}
	}

	return 0;
}
