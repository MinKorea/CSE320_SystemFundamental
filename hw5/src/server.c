#include "server.h"
#include "protocol.h"
#include "store.h"
#include "csapp.h"
#include "debug.h"

CLIENT_REGISTRY *client_registry;

void *xacto_client_service(void *arg)
{
	int connfd = *((int *)arg);
	Free(arg);
	Pthread_detach(pthread_self());

	creg_register(client_registry, connfd);

	TRANSACTION *new_trans = trans_create();

	while(1)
	{
		XACTO_PACKET *pkt = Malloc(sizeof(XACTO_PACKET));
		void **datap = Malloc(sizeof(void *) * 3);

		if(proto_recv_packet(connfd, pkt, datap) == 0)
		{

			debug("PKT TYPE: %d", pkt->type);
			// free(*datap);
			datap++;

			if(pkt->type == XACTO_PUT_PKT)
			{
				debug("PUT packet received");
				proto_recv_packet(connfd, pkt, datap);

				int key_size = ntohl(pkt->size);
				debug("Received key, size %d", key_size);

				char *cli_key_str = (char *)(*datap);
				// free(*datap);
				datap++;

				// debug("Key: %s", cli_key_str);

				proto_recv_packet(connfd,pkt, datap);

				int value_size = ntohl(pkt->size);
				debug("Received value, size %d", value_size);
				char *cli_value = (char *)(*datap);

				BLOB* key_blob = blob_create(cli_key_str, key_size);
				KEY* cli_key = key_create(key_blob);
				BLOB* value_blob = blob_create(cli_value, value_size);
				// debug("Value: %s", cli_value);

				int trans_status = store_put(new_trans, cli_key, value_blob);

				pkt->type = XACTO_REPLY_PKT;
				pkt->status = trans_status;

				proto_send_packet(connfd, pkt, *datap);

				free(*datap);


			}
			else if(pkt->type == XACTO_GET_PKT)
			{
				debug("GET packet received");
				proto_recv_packet(connfd, pkt, datap);

				int key_size = ntohl(pkt->size);
				debug("Received key, size %d", key_size);
				char *cli_key_str = (char *)(*datap);
				BLOB* key_blob = blob_create(cli_key_str, key_size);
				KEY* cli_key = key_create(key_blob);
				BLOB** value_p = malloc(sizeof(BLOB *));

				int trans_status = store_get(new_trans, cli_key, value_p);

				if(*value_p == NULL)
				{
					pkt->type = XACTO_REPLY_PKT;
					pkt->status = trans_status;
					pkt->size = 0;

					proto_send_packet(connfd, pkt, NULL);

					pkt->type = XACTO_VALUE_PKT;

					pkt->null = 1;
					proto_send_packet(connfd, pkt, NULL);
				}
				else
				{
					pkt->type = XACTO_REPLY_PKT;
					pkt->status = trans_status;

					proto_send_packet(connfd, pkt, (*value_p)->content);

					pkt->type = XACTO_VALUE_PKT;
					pkt->size = htonl((*value_p)->size);
					proto_send_packet(connfd, pkt, (*value_p)->content);
				}

				free(*datap);

			}
			else if(pkt->type == XACTO_COMMIT_PKT)
			{
				debug("COMMIT packet received");
				int trans_status = trans_commit(new_trans);

				pkt->type = XACTO_REPLY_PKT;
				pkt->status = trans_status;

				proto_send_packet(connfd, pkt, NULL);

				break;

			}

		}
		else
		{
			trans_abort(new_trans);
			break;
		}
	}

	creg_unregister(client_registry, connfd);

	Close(connfd);
	return NULL;
}