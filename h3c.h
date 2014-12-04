/*
 * h3c.h
 *
 *  Created on: Dec 2, 2014
 *      Author: baoke
 */

#ifndef H3C_H_
#define H3C_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define EAPOL_VERSION 1

#define EAP_TYPE_ID 1
#define EAP_TYPE_MD5 4
#define EAP_TYPE_H3C 7

#define EAPOL_EAPPACKET 0
#define EAPOL_START 1
#define EAPOL_LOGOFF 2
#define EAPOL_KEY 3
#define EAPOL_ASF 4

#define EAP_REQUEST 1
#define EAP_RESPONSE 2
#define EAP_SUCCESS 3
#define EAP_FAILURE 4

#define BUF_LEN 256
#define MD5_LEN 16

struct eapol{
	unsigned char version;
	unsigned char type;
	unsigned short p_len;
}__attribute__ ((packed)) eapol;

struct eap{
	unsigned char code;
	unsigned char id;
	unsigned short d_len;
	unsigned char type;
}__attribute__ ((packed)) eap;

struct packet{
	struct ether_header eth_header;
	struct eapol eapol_header;
	struct eap eap_header;
}__attribute__ ((packed)) packet;

#endif /* H3C_H_ */
