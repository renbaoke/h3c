/*
 * h3c.c
 *
 *  Created on: Dec 2, 2014
 *      Author: baoke
 */

#include "h3c.h"

int sockfd;

char *interface;
char *username;
char *password;

const char PAE_GROUP_ADDR[] = \
		{0x01, 0x80, 0xc2, 0x00, 0x00, 0x03};

const char VERSION_INFO[] = \
		{0x06, 0x07, 'b', 'j', 'Q', '7', 'S', 'E', '8', 'B', 'Z', '3', 'M', \
	'q', 'H', 'h', 's', '3', 'c', 'l', 'M', 'r', 'e', 'g', 'c', 'D', 'Y', \
	'3', 'Y', '=',0x20,0x20};

unsigned char send_buf[BUF_LEN];
unsigned char recv_buf[BUF_LEN];

struct sockaddr_ll addr;

void h3c_set_eapol_header(unsigned char type, unsigned short p_len)
{
	struct packet *pkt;
	pkt = (struct packet *)send_buf;

	pkt->eapol_header.version = EAPOL_VERSION;
	pkt->eapol_header.type = type;
	pkt->eapol_header.p_len = p_len;
}

void h3c_set_eap_header(unsigned char code, unsigned char id, \
		unsigned short d_len, unsigned char type)
{
	struct packet *pkt;
	pkt = (struct packet *)send_buf;

	pkt->eap_header.code = code;
	pkt->eap_header.id = id;
	pkt->eap_header.d_len = d_len;
	pkt->eap_header.type = type;
}

void h3c_init()
{
	struct ifreq ifr;
	struct packet *pkt;
	pkt = (struct packet *)send_buf;

	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_PAE));
	if (-1 == sockfd)
		exit(errno);

	memcpy(pkt->eth_header.ether_dhost,PAE_GROUP_ADDR,ETH_ALEN);

	strcpy(ifr.ifr_name, interface);
	if (-1 == ioctl(sockfd, SIOCGIFHWADDR, &ifr))
		exit(errno);

	memcpy(pkt->eth_header.ether_shost,ifr.ifr_hwaddr.sa_data,ETH_ALEN);

	if(-1 == ioctl(sockfd,SIOCGIFINDEX,&ifr))
		exit(errno);

	addr.sll_ifindex = ifr.ifr_ifindex;

	/*
	 * use htons when the data is more than 1 byte
	 */
	pkt->eth_header.ether_type = htons(ETH_P_PAE);
}

void h3c_start()
{
	h3c_set_eapol_header(EAPOL_START, 0);

	if (-1 == sendto(sockfd, (void *)send_buf, sizeof(struct ether_header) + \
			sizeof(struct eapol), 0, (struct sockaddr*)&addr, sizeof(addr)))
		exit(errno);
}

void h3c_logoff()
{
	h3c_set_eapol_header(EAPOL_LOGOFF, 0);

	if (-1 == sendto(sockfd, (void *)send_buf, sizeof(struct ether_header) + \
			sizeof(struct eapol), 0, (struct sockaddr*)&addr, sizeof(addr)))
		exit(errno);
}

void h3c_send_id(unsigned char packet_id)
{
	unsigned short len = htons(sizeof(struct eap) + sizeof(VERSION_INFO) + \
			strlen(username));
	unsigned char *data = (unsigned char *)(send_buf + sizeof(struct packet));

	h3c_set_eapol_header(EAPOL_EAPPACKET, len);
	h3c_set_eap_header(EAP_RESPONSE, packet_id, len, EAP_TYPE_ID);

	memcpy(data, VERSION_INFO, sizeof(VERSION_INFO));

	data += sizeof(VERSION_INFO);
	memcpy(data, username, strlen(username));

	if (-1 == sendto(sockfd, (void *)send_buf, sizeof(struct packet) + \
			sizeof(VERSION_INFO) + strlen(username), 0, \
			(struct sockaddr*)&addr, sizeof(addr)))
		exit(errno);
}

void h3c_send_md5(unsigned char packet_id, unsigned char *md5data)
{
	unsigned char md5[MD5_LEN];
	unsigned short len = htons(sizeof(struct eap) + 1 + MD5_LEN + strlen(username));
	unsigned char *data = (unsigned char *)(send_buf + sizeof(struct packet));

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

	if (-1 == sendto(sockfd, (void *)send_buf, sizeof(struct packet) + 1 + \
			MD5_LEN + strlen(username), 0, (struct sockaddr*)&addr, sizeof(addr)))
		exit(errno);
}

void h3c_send_h3c(unsigned char packet_id)
{
	size_t password_size = strlen(password);
	size_t username_size = strlen(username);

	unsigned short len = htons(sizeof(struct eap) + password_size + username_size);
	unsigned char *data = (unsigned char *)(send_buf + sizeof(struct packet));

	h3c_set_eapol_header(EAPOL_EAPPACKET, len);
	h3c_set_eap_header(EAP_RESPONSE, packet_id, len, EAP_TYPE_H3C);

	*data = (unsigned char)password_size;

	data++;
	memcpy(data, password, password_size);

	data += strlen(password);
	memcpy(data, username, username_size);

	if (-1 == sendto(sockfd, (void *)send_buf, sizeof(struct packet) + 1 + \
			password_size + username_size, 0, (struct sockaddr*)&addr, sizeof(addr)))
		exit(errno);
}

void h3c_response()
{
	struct packet *pkt;
	pkt = (struct packet *)recv_buf;

	socklen_t len;
	len = sizeof(addr);

	if (-1 == recvfrom(sockfd, (void *)recv_buf, BUF_LEN, 0, \
			(struct sockaddr *)&addr, &len))
		exit(errno);

	if (pkt->eapol_header.type != EAPOL_EAPPACKET)
	{
		/*
		 * Got unknown eapol type
		 */
		return;
	}

	if (pkt->eap_header.code == EAP_SUCCESS)
	{
		/*
		 * Got success
		 * Go daemon
		 */
		daemon(0,0);
	}
	else if (pkt->eap_header.code == EAP_FAILURE)
	{
		/*
		 * Got failure
		 */
		exit(-1);
	}
	else if (pkt->eap_header.code == EAP_REQUEST)
	{
		/*
		 * Got request
		 * Response according request type
		 */
		if (pkt->eap_header.type == EAP_TYPE_ID)
		{
			/*
			 * Response id
			 */
			h3c_send_id(pkt->eap_header.id);
		}
		else if (pkt->eap_header.type == EAP_TYPE_MD5)
		{
			/*
			 * Response md5
			 */
			unsigned char *md5;
			md5 = (unsigned char *)(recv_buf + sizeof(struct packet) + 1);
			h3c_send_md5(pkt->eap_header.id, md5);
		}
		else if (pkt->eap_header.type == EAP_TYPE_H3C)
		{
			/*
			 * Response h3c
			 */
			h3c_send_h3c(pkt->eap_header.id);
		}
	}
	else if (pkt->eap_header.code == EAP_RESPONSE)
	{
		/*
		 * Got response
		 */
	}
}

int main(int argc, char **argv)
{
	/*
	 * Check privilege
	 */
	if (0 != getuid())
	{
		printf("please run as root\n");
		exit(-1);
	}

	/*
	 * Check arguments
	 */
	if (4 != argc)
	{
		printf("usage: %s [interface] [username] [password]\n", argv[0]);
		exit(-1);
	}

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
