/*
 * main.c
 *
 *  Created on: Dec 5, 2014
 *      Author: baoke
 */

#include "h3c.h"

int sockfd;

char *interface;
char *username;
char *password;

unsigned char send_buf[BUF_LEN];
unsigned char recv_buf[BUF_LEN];

struct sockaddr_ll addr;

int main(int argc, char **argv)
{
	/*
	 * Check privilege
	 */
	if (0 != geteuid())
		exit(-1);

	/*
	 * Check arguments
	 */
	if (4 != argc)
		exit(-1);

	interface = argv[1];
	username = argv[2];
	password = argv[3];

	h3c_init();

	h3c_start();

	for(;;)
	{
		h3c_response();
	}

	return 0;
}
