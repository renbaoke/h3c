/*
 * h3c.c
 *
 *  Created on: Dec 2, 2014
 *      Author: baoke
 */

#include "h3c.h"

static int sockfd;

static char *interface;
static char *username;
static char *password;

static unsigned char send_buf[BUF_LEN];
static unsigned char recv_buf[BUF_LEN];

static struct sockaddr_ll addr;

static void h3c_set_eapol_header(unsigned char type, unsigned short p_len)
{
	struct packet *pkt;
	pkt = (struct packet *)send_buf;

	pkt->eapol_header.version = EAPOL_VERSION;
	pkt->eapol_header.type = type;
	pkt->eapol_header.p_len = p_len;
}

static void h3c_set_eap_header(unsigned char code, unsigned char id, \
		unsigned short d_len, unsigned char type)
{
	struct packet *pkt;
	pkt = (struct packet *)send_buf;

	pkt->eap_header.code = code;
	pkt->eap_header.id = id;
	pkt->eap_header.d_len = d_len;
	pkt->eap_header.type = type;
}

static int h3c_send_id(unsigned char packet_id)
{
	unsigned short len = htons(sizeof(struct eap) + \
			sizeof(VERSION_INFO) + strlen(username));
	unsigned char *data = \
			(unsigned char *)(send_buf + sizeof(struct packet));

	h3c_set_eapol_header(EAPOL_EAPPACKET, len);
	h3c_set_eap_header(EAP_RESPONSE, packet_id, len, EAP_TYPE_ID);

	memcpy(data, VERSION_INFO, sizeof(VERSION_INFO));

	data += sizeof(VERSION_INFO);
	memcpy(data, username, strlen(username));

	return sendto(sockfd, (void *)send_buf, sizeof(struct packet) + \
			sizeof(VERSION_INFO) + strlen(username), 0, \
			(struct sockaddr*)&addr, sizeof(addr));
}

static int h3c_send_md5(unsigned char packet_id, unsigned char *md5data)
{
	unsigned char md5[MD5_LEN];
	unsigned short len = htons(sizeof(struct eap) + 1 + \
			MD5_LEN + strlen(username));
	unsigned char *data = \
			(unsigned char *)(send_buf + sizeof(struct packet));

	memset(md5, 0, MD5_LEN);
	memcpy(md5, password, strlen(password));
	int i;
	for (i = 0; i < MD5_LEN; i++)
		md5[i] ^= md5data[i];

	h3c_set_eapol_header(EAPOL_EAPPACKET, len);
	h3c_set_eap_header(EAP_RESPONSE, packet_id, len, EAP_TYPE_MD5);

	*data = MD5_LEN;

	data++;
	memcpy(data, md5, MD5_LEN);

	data += MD5_LEN;
	memcpy(data, username, strlen(username));

	return sendto(sockfd, (void *)send_buf, sizeof(struct packet) + \
			1 + MD5_LEN + strlen(username), 0, \
			(struct sockaddr*)&addr, sizeof(addr));
}

static int h3c_send_h3c(unsigned char packet_id)
{
	/*
	 * not called so far
	 */
	size_t password_size = strlen(password);
	size_t username_size = strlen(username);

	unsigned short len = htons(sizeof(struct eap) + password_size + \
			username_size);
	unsigned char *data = \
			(unsigned char *)(send_buf + sizeof(struct packet));

	h3c_set_eapol_header(EAPOL_EAPPACKET, len);
	h3c_set_eap_header(EAP_RESPONSE, packet_id, len, EAP_TYPE_H3C);

	*data = (unsigned char)password_size;

	data++;
	memcpy(data, password, password_size);

	data += strlen(password);
	memcpy(data, username, username_size);

	return sendto(sockfd, (void *)send_buf, sizeof(struct packet) + \
			1 + password_size + username_size, 0, \
			(struct sockaddr*)&addr, sizeof(addr));
}

int h3c_init(char *_interface, char *_username, char *_password)
{
	struct ifreq ifr;
	struct packet *pkt;
	pkt = (struct packet *)send_buf;

	interface = _interface;
	username = _username;
	password = _password;

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_PAE));
	if (-1 == sockfd)
		return -1;

	memcpy(pkt->eth_header.ether_dhost,PAE_GROUP_ADDR,ETH_ALEN);

	strcpy(ifr.ifr_name, interface);
	if (-1 == ioctl(sockfd, SIOCGIFHWADDR, &ifr))
		return -1;

	memcpy(pkt->eth_header.ether_shost,ifr.ifr_hwaddr.sa_data,ETH_ALEN);

	if(-1 == ioctl(sockfd,SIOCGIFINDEX,&ifr))
		return -1;

	addr.sll_ifindex = ifr.ifr_ifindex;

	/*
	 * use htons when the data is more than 1 byte
	 */
	pkt->eth_header.ether_type = htons(ETH_P_PAE);
	
	return 0;
}

int h3c_start()
{
	h3c_set_eapol_header(EAPOL_START, 0);

	return sendto(sockfd, (void *)send_buf, \
			sizeof(struct ether_header)+ sizeof(struct eapol), 0, \
			(struct sockaddr*)&addr, sizeof(addr));
}

int h3c_logoff()
{
	h3c_set_eapol_header(EAPOL_LOGOFF, 0);

	return sendto(sockfd, (void *)send_buf, \
			sizeof(struct ether_header) + sizeof(struct eapol), 0, \
			(struct sockaddr*)&addr, sizeof(addr));
}

int h3c_response(int (*success_callback)(), int (*failure_callback)())
{
	struct packet *pkt;
	pkt = (struct packet *)recv_buf;

	socklen_t len;
	len = sizeof(addr);

	if (-1 == recvfrom(sockfd, (void *)recv_buf, BUF_LEN, 0, \
			(struct sockaddr *)&addr, &len))
		return -1;

	if (pkt->eapol_header.type != EAPOL_EAPPACKET)
		/*
		 * Got unknown eapol type
		 */
		return 0;

	if (pkt->eap_header.code == EAP_SUCCESS)
	{
		/*
		 * Got success
		 */
		if (NULL != success_callback)
			return (*success_callback)();
		else
			return 0;
	}
	else if (pkt->eap_header.code == EAP_FAILURE)
	{
		/*
		 * Got failure
		 */
		if (NULL != *failure_callback)
			return (*failure_callback)();
		else
			return 0;
	}
	else if (pkt->eap_header.code == EAP_REQUEST)
	{
		/*
		 * Got request
		 * Response according to request type
		 */
		if (pkt->eap_header.type == EAP_TYPE_ID)
			return h3c_send_id(pkt->eap_header.id);
		else if (pkt->eap_header.type == EAP_TYPE_MD5)
		{
			unsigned char *md5;
			md5 = (unsigned char *)(recv_buf + sizeof(struct packet) + 1);
			return h3c_send_md5(pkt->eap_header.id, md5);
		}
		else if (pkt->eap_header.type == EAP_TYPE_H3C)
			return h3c_send_h3c(pkt->eap_header.id);
	}
	else if (pkt->eap_header.code == EAP_RESPONSE)
		/*
		 * Got response
		 */
		return 0;

	return 0;
}
