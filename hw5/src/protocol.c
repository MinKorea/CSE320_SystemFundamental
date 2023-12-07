#include "protocol.h"
#include "csapp.h"

#include <stdio.h>
#include "debug.h"


int proto_send_packet(int fd, XACTO_PACKET *pkt, void *data)
{

	if(rio_writen(fd, pkt, sizeof(XACTO_PACKET)) == -1)
		return -1;

	int payload_size = htonl(pkt->size);

	// pkt->serial = htonl(pkt->serial);
	// pkt->size = htonl(pkt->size);
	// pkt->timestamp_sec = htonl(pkt->timestamp_sec);
	// pkt->timestamp_nsec = htonl(pkt->timestamp_nsec);

	// debug("PAYLOAD_SIZE: %d", payload_size);

	if(payload_size > 0)
	{

		if(rio_writen(fd, data, payload_size) == -1)
		{
			debug("Returned -1");
			Free(data);
			return -1;
		}
	}
	// else
	// {
	// 	if(rio_writen(fd, NULL, pkt->null) == -1)
	// 	{
	// 		debug("Returned -1");
	// 		return -1;
	// 	}
	// }

	debug("DEBUG_SEND_PKT");
	// printf("fd: %d\n", fd);

	return 0;
}

int proto_recv_packet(int fd, XACTO_PACKET *pkt, void **datap)
{

	if(rio_readn(fd, pkt, sizeof(XACTO_PACKET)) <= 0)
		return -1;

	int payload_size = ntohl(pkt->size);

	// pkt->serial = ntohl(pkt->serial);
	// pkt->size = ntohl(pkt->size);
	// pkt->timestamp_sec = ntohl(pkt->timestamp_sec);
	// pkt->timestamp_nsec = ntohl(pkt->timestamp_nsec);

	debug("PAYLOAD_SIZE: %d", payload_size);

	if(pkt->size > 0)
	{
		*datap = Malloc(payload_size + 1);

		if(rio_readn(fd, *datap, payload_size) <= 0)
		{
			debug("Returned -1");
			Free(*datap);
			return -1;
		}
	}
	// else
	// {
	// 	if(rio_readn(fd, NULL, pkt->null) == -1)
	// 	{
	// 		debug("Returned -1");
	// 		return -1;
	// 	}
	// }


	debug("DEBUG_RECEIVE_PKT");
	// printf("fd: %d\n", fd);

	return 0;
}