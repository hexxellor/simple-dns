#include "sllp-util.h"

int32_t sllp_process_request_echo(struct sllp_server* server, struct sllp_request* request)
{
    int32_t datalen;

    memset(request->data, 0, request->bufsize);
    datalen = request->sock->recv(request->sock, request->data, request->bufsize, 0);
    if (!datalen)
	return -1;

    printf("receive data from: (%s, %d), len: %d, data: %s\n", 
	   request->sock->address.host, request->sock->address.port, datalen, request->data);
    request->sock->sendall(request->sock, request->data, datalen, 0);

    return 0;
}

int32_t sllp_dummy_handle_error(struct sllp_server* server, struct sllp_request* request)
{
    /* TODO: record an error item on the log system;
     *	     dump the request information.
     */
    return 0;
}


int32_t sllp_dummy_handle_timeout(struct sllp_server* server)
{
    /* TODO: nothing to do */
    return 0;
}


int32_t sllp_dummy_verify_request(struct sllp_server* server, struct sllp_request* request)
{
    /* TODO: record an verify error item on the log system;
     *
     */
    return 0;
}
