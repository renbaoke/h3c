/*
 * handler.c
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

#include "handler.h"
#include "h3c.h"
#include "echo.h"

int success_handler() {
	printf("You are now ONLINE.\n");
	daemon(0, 0);
	return SUCCESS;
}

int failure_handler() {
	printf("You are now OFFLINE.\n");
	return SUCCESS;
}

int unkown_eapol_handler() {
	return SUCCESS;
}

int unkown_eap_handler() {
	return SUCCESS;
}

/* we should NOT got response messages */
int got_response_handler() {
	return SUCCESS;
}

void exit_handler(int arg) {
	puts("\nExiting...\n");
	h3c_logoff();
	h3c_clean();
	exit(0);
}

void exit_with_echo_on(int arg) {
	putchar('\n');
	echo_on();
	exit(0);
}
