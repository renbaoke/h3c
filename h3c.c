/*
 * h3c.c
 *
 *  Created on: Dec 2, 2014
 *      Author: baoke
 */

#include "h3c.h"

#define pkt(p) ((struct packet *)(p))

#define eap_type(p) ((unsigned char *)(p) + sizeof(struct packet))
#define eap_data(p) (eap_type(p) + TYPE_LEN)

#define eap_id_info(p) eap_data(p)
#define eap_id_username(p) ((eap_id_info(p)) + sizeof(VERSION_INFO))

#define eap_md5_length(p) eap_data(p)
#define eap_md5_data(p) ((eap_md5_length(p)) + MD5_LEN_LEN)
#define eap_md5_username(p) ((eap_md5_data(p)) + MD5_LEN)

#define eap_h3c_length(p) eap_data(p)
#define eap_h3c_password(p) ((eap_h3c_length(p)) + H3C_LEN_LEN)
#define eap_h3c_username(p) (eap_h3c_password(p) + password_length)

static int sockfd;

static char *username, *password;
static int username_length, password_length;

static unsigned char send_buf[BUF_LEN];
static unsigned char recv_buf[BUF_LEN];

static struct sockaddr_ll addr;

static void set_eapol_header(unsigned char type, \
		unsigned short length)
{
	pkt(send_buf)->eapol_header.version = EAPOL_VERSION;
	pkt(send_buf)->eapol_header.type = type;
	pkt(send_buf)->eapol_header.length = length;
}

static void set_eap_header(unsigned char code, unsigned char id, \
		unsigned short length)
{
	pkt(send_buf)->eap_header.code = code;
	pkt(send_buf)->eap_header.id = id;
	pkt(send_buf)->eap_header.length = length;
}

static int sendout(int length)
{
	return sendto(sockfd, (void *)send_buf, length, 0, \
			(struct sockaddr*)&addr, sizeof(addr));
}

static int send_id(unsigned char packet_id)
{
	unsigned short len = htons( sizeof(struct eap) + TYPE_LEN + \
			sizeof(VERSION_INFO) + username_length );

	set_eapol_header(EAPOL_EAPPACKET, len);
	set_eap_header(EAP_RESPONSE, packet_id, len);
	*eap_type(send_buf) = EAP_TYPE_ID;

	memcpy(eap_id_info(send_buf), VERSION_INFO, sizeof(VERSION_INFO));
	memcpy(eap_id_username(send_buf), username, username_length);

	return sendout(sizeof(struct packet) + TYPE_LEN + \
			sizeof(VERSION_INFO) + username_length);
}

static int send_md5(unsigned char packet_id, unsigned char *md5data)
{
	unsigned char md5[MD5_LEN];
	unsigned short len = htons(sizeof(struct eap) + TYPE_LEN + \
			MD5_LEN_LEN + MD5_LEN + username_length);

	memset(md5, 0, MD5_LEN);
	strcpy(md5, password);

	int i;
	for (i = 0; i < MD5_LEN; i++)
		md5[i] ^= md5data[i];

	set_eapol_header(EAPOL_EAPPACKET, len);
	set_eap_header(EAP_RESPONSE, packet_id, len);
	*eap_type(send_buf) = EAP_TYPE_MD5;

	*eap_md5_length(send_buf) = MD5_LEN;
	memcpy(eap_md5_data(send_buf), md5, MD5_LEN);
	memcpy(eap_md5_username(send_buf), username, username_length);

	return sendout(sizeof(struct packet) + TYPE_LEN + MD5_LEN_LEN + \
			MD5_LEN + username_length);
}

static int send_h3c(unsigned char packet_id)
{
	/*
	 * not called so far
	 */
	unsigned short len = htons(sizeof(struct eap) + 1 + 1 + \
			password_length + username_length);

	set_eapol_header(EAPOL_EAPPACKET, len);
	set_eap_header(EAP_RESPONSE, packet_id, len);
	*eap_type(send_buf) = EAP_TYPE_H3C;

	*eap_h3c_length(send_buf) = (unsigned char)password_length;
	memcpy(eap_h3c_password(send_buf), password, password_length);
	memcpy(eap_h3c_username(send_buf), username, username_length);

	return sendout(sizeof(struct packet) + TYPE_LEN + H3C_LEN_LEN + \
			password_length + username_length);
}

int h3c_init(char *_interface)
{
	struct ifreq ifr;

	if (-1 == (sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_PAE))))
		return -1;

	strcpy(ifr.ifr_name, _interface);
	if (-1 == ioctl(sockfd,SIOCGIFINDEX,&ifr))
		return -1;
	else
		addr.sll_ifindex = ifr.ifr_ifindex;

	//copy the group address to ether header
	memcpy(pkt(send_buf)->eth_header.ether_dhost, PAE_GROUP_ADDR, \
			ETH_ALEN);

	//get the interface mac address
	if (-1 == ioctl(sockfd, SIOCGIFHWADDR, &ifr))
		return -1;

	//copy the interface mac address to ether header
	memcpy(pkt(send_buf)->eth_header.ether_shost, \
			ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	//set the ether type
	pkt(send_buf)->eth_header.ether_type = htons(ETH_P_PAE);

	return 0;
}

void set_username(char *_username)
{
	username_length = strlen(_username);
	username = (char *)malloc(username_length);
	strcpy(username, _username);
}

void set_password(char *_password)
{
	password_length = strlen(_password);
	password = (char *)malloc(password_length);
	strcpy(password, _password);
}

int start()
{
	set_eapol_header(EAPOL_START, 0);
	return sendout(sizeof(struct ether_header)+ sizeof(struct eapol));
}

int logoff()
{
	set_eapol_header(EAPOL_LOGOFF, 0);
	return sendout(sizeof(struct ether_header)+ sizeof(struct eapol));
}

int response(int (*success_callback)(), int (*failure_callback)())
{
	socklen_t len;
	len = sizeof(addr);

	if (-1 == recvfrom(sockfd, (void *)recv_buf, BUF_LEN, 0, \
			(struct sockaddr *)&addr, &len))
		return -1;

	if (pkt(recv_buf)->eapol_header.type != EAPOL_EAPPACKET)
		/*
		 * Got unknown eapol type
		 */
		return 0;

	if (pkt(recv_buf)->eap_header.code == EAP_SUCCESS)
		/*
		 * Got success
		 */
		if (NULL != success_callback)
			return success_callback();
		else
			return 0;
	else if (pkt(recv_buf)->eap_header.code == EAP_FAILURE)
		/*
		 * Got failure
		 */
		if (NULL != *failure_callback)
			return failure_callback();
		else
			return 0;
	else if (pkt(recv_buf)->eap_header.code == EAP_REQUEST)
		/*
		 * Got request
		 * Response according to request type
		 */
		if (*eap_type(recv_buf) == EAP_TYPE_ID)
			return send_id(pkt(recv_buf)->eap_header.id);
		else if (*eap_type(recv_buf) == EAP_TYPE_MD5)
			return send_md5(pkt(recv_buf)->eap_header.id, \
					eap_md5_data(recv_buf));
		else if (*eap_type(recv_buf) == EAP_TYPE_H3C)
			return send_h3c(pkt(recv_buf)->eap_header.id);
	else if (pkt(recv_buf)->eap_header.code == EAP_RESPONSE)
		/*
		 * Got response
		 */
		return 0;

	return 0;
}

void clean()
{
	free(username);
	free(password);
	//close(sockfd);
}
