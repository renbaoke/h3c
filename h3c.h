/*
 * h3c.h
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

#ifndef H3C_H_
#define H3C_H_

#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

#ifdef AF_LINK
/* BSD */
#include <ifaddrs.h>
#include <net/if_dl.h>
#include <net/bpf.h>
#include <net/if_types.h>
#include <fcntl.h>

#ifdef __NetBSD__
#include <net/if_ether.h>
#elif __FreeBSD__
#include <net/ethernet.h>
#elif __OpenBSD__
#include <netinet/if_ether.h>
#else
#error UNKOWN UNIX PLATFORM
#endif

#else
/* Linux */
#include <netpacket/packet.h>
#include <net/ethernet.h>
#endif /* AF_LINK */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define EAPOL_VERSION 1

#define EAPOL_EAPPACKET 0
#define EAPOL_START 1
#define EAPOL_LOGOFF 2
#define EAPOL_KEY 3
#define EAPOL_ASF 4

#define EAP_TYPE_ID 1
#define EAP_TYPE_MD5 4
#define EAP_TYPE_H3C 7

#define EAP_REQUEST 1
#define EAP_RESPONSE 2
#define EAP_SUCCESS 3
#define EAP_FAILURE 4

#define BUF_LEN 256
#define MSG_LEN 32
#define MD5_LEN 16
#define USR_LEN 16
#define PWD_LEN 16
#define TYPE_LEN 1
#define MD5_LEN_LEN 1
#define H3C_LEN_LEN 1

#ifndef ETH_P_PAE
#define ETH_P_PAE 0x888E
#endif

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

/* Function Status */
#define SUCCESS 0
#define USR_TOO_LONG 1
#define PWD_TOO_LONG 2
#define BPF_OPEN_ERR 3
#define BPF_SET_BUF_LEN_ERR 4
#define BPF_SET_IF_ERR 5
#define BPF_SET_FILTER_ERR 6
#define BPF_SET_IMMEDIATE_ERR 7
#define BPF_SET_DIRECTION_ERR 8
#define SOCKET_OPEN_ERR 9
#define SOCKET_SET_IF_ERR 10
#define SOCKET_GET_HWADDR_ERR 11
#define SEND_ERR 12
#define RECV_ERR 13
#define EAPOL_UNHANDLED 14
#define EAP_UNHANDLED 15
#define SUCCESS_UNHANDLED 16
#define FAILURE_UNHANDLED 17
#define RESPONSE_UNHANDLED 18

struct eapol{
	unsigned char version;
	unsigned char type;
	unsigned short length;
}__attribute__ ((packed)) eapol;

struct eap{
	unsigned char code;
	unsigned char id;
	unsigned short length;
}__attribute__ ((packed)) eap;

struct packet{
	struct ether_header eth_header;
	struct eapol eapol_header;
	struct eap eap_header;
}__attribute__ ((packed)) packet;

const static char PAE_GROUP_ADDR[] = \
		{0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};/* broadcast mac address */

const static char VERSION_INFO[] = \
		{0x06, 0x07, 'b', 'j', 'Q', '7', 'S', 'E', '8', 'B', 'Z', '3', \
		'M', 'q', 'H', 'h', 's', '3', 'c', 'l', 'M', 'r', 'e', 'g', \
		'c', 'D', 'Y', '3', 'Y', '=',0x20,0x20};/* learned from yah3c */

/*
 * param _interface: ethernet device name, e.g. eth0.
 * 
 * return:
 * 
 * SUCCESS
 * BPF_OPEN_ERR
 * BPF_SET_BUF_LEN_ERR
 * BPF_SET_IF_ERR
 * BPF_SET_FILTER_ERR
 * BPF_SET_IMMEDIATE_ERR
 * BPF_SET_DIRECTION_ERR
 * SOCKET_OPEN_ERR
 * SOCKET_SET_IF_ERR
 * SOCKET_GET_HWADDR_ERR
 */
int h3c_init(char *_interface);

/*
 * return:
 * 
 * SUCCESS
 * SEND_ERR
 */
int h3c_start();
int h3c_logoff();

/*
 * return:
 * 
 * SUCCESS
 * RECV_ERR
 * SEND_ERR
 */
int h3c_response(int (*success_callback)(void), \
	int (*failure_callback)(void), \
	int (*unkown_eapol_callback)(void), \
	int (*unkown_eap_callback)(void), \
	int (*got_response_callback)(void));

/*
 * return:
 * 
 * SUCCESS
 * USR_TOO_LONG
 */
int h3c_set_username(char *_username);

/*
 * return:
 * 
 * SUCCESS
 * PWD_TOO_LONG
 */
int h3c_set_password(char *_password);

void h3c_clean();

#endif /* H3C_H_ */
