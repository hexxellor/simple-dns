#include "sllp-util.h"

int32_t sllp_process_request_echo(struct sllp_server* server, struct sllp_socket* request)
{
    u_int8_t buf[1024];
    u_int32_t datalen;

    memset(buf, 0, sizeof(buf));
    datalen = request->recv(request, buf, sizeof(buf), 0);
    if (!datalen)
	return -1;

    printf("receive data from: (%s, %d), len: %d, data: %s\n", request->address.host, request->address.port, datalen, buf);
    request->sendall(request, buf, datalen, 0);

    return 0;
}

int32_t sllp_handle_error(struct sllp_server* server, struct sllp_socket* request)
{
    /* TODO: record an error item on the log system;
     *	     dump the request information.
     */
    return 0;
}


int32_t sllp_handle_timeout(struct sllp_server* server)
{
    /* TODO: nothing to do */
    return 0;
}


int32_t sllp_verify_request(struct sllp_server* server, struct sllp_socket* request)
{
    /* TODO: record an verify error item on the log system;
     *
     */
    return 0;
}
