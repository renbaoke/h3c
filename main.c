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
#include "echo.h"
#include "h3c.h"
#include "handler.h"

void usage(FILE *stream);

int main(int argc, char **argv) {
	int ch;
	char *interface = NULL;
	char *username = NULL;
	char *password = NULL;
	int alloc_pw = 0;

	while ((ch = getopt(argc, argv, "i:u:p:h")) != -1) {
		switch (ch) {
		case 'i':
			interface = optarg;
			break;
		case 'u':
			username = optarg;
			break;
		case 'p':
			password = optarg;
			break;
		case 'h':
			usage(stdout);
			exit(0);
		default:
			usage(stderr);
			exit(-1);
		}
	}

	/* must run as root */
	if (geteuid() != 0) {
		fprintf(stderr, "Run as root, please.\n");
		exit(-1);
	}

	if (interface == NULL || username == NULL) {
		usage(stderr);
		exit(-1);
	}

	if (h3c_set_username(username) != SUCCESS) {
		fprintf(stderr, "Failed to set username.\n");
		exit(-1);
	}

	if (password == NULL) {
		if ((password = (char *) malloc(PWD_LEN)) == NULL) {
			fprintf(stderr, "Failed to malloc: %s\n", strerror(errno));
			exit(-1);
		}
		alloc_pw = 1;
		printf("Password for %s:", username);

		signal(SIGINT, exit_with_echo_on);
		signal(SIGTERM, exit_with_echo_on);

		echo_off();
		fgets(password, PWD_LEN - 1, stdin);
		echo_on();

		/* replace '\n' with '\0', as it is NOT part of password */
		password[strlen(password) - 1] = '\0';
		putchar('\n');
	}

	if (h3c_set_password(password) != SUCCESS) {
		fprintf(stderr, "Failed to set password.\n");
		if (alloc_pw) free(password);
		exit(-1);
	}
	if (alloc_pw) free(password);

	if (h3c_init(interface) != SUCCESS) {
		fprintf(stderr, "Failed to initialize: %s\n", strerror(errno));
		exit(-1);
	}

	if (h3c_start() != SUCCESS) {
		fprintf(stderr, "Failed to start: %s\n", strerror(errno));
		exit(-1);
	}

	signal(SIGINT, exit_handler);
	signal(SIGTERM, exit_handler);

	for (;;) {
		if (h3c_response(success_handler, failure_handler, unkown_eapol_handler,
				unkown_eap_handler, got_response_handler) != SUCCESS) {
			fprintf(stderr, "Failed to response: %s\n", strerror(errno));
			exit(-1);
		}
	}

	return 0;
}

void usage(FILE *stream) {
	fprintf(stream, "Usage: h3c [OPTION]...\n");
	fprintf(stream, "  -i <interface>\tspecify interface, required\n");
	fprintf(stream, "  -u <username>\t\tspecify username, required\n");
	fprintf(stream, "  -p <password>\t\tspecify password, optional\n");
	fprintf(stream, "  -h\t\t\tshow this message\n");
}

