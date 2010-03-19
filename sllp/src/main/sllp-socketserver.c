#include "sllp-socketserver.h"
#include "sllp-select.h"

/* function declare */
struct sllp_server* internal_create_server(int32_t type);
void internal_free_server(struct sllp_server* server);
int32_t server_server_forever(struct sllp_server *server, int32_t poll_interval);
int32_t server_shutdown(struct sllp_server* server);
int32_t server_handle_request(struct sllp_server* server);

int32_t server__handle_request_noblock(struct sllp_server *server);
int32_t server__server_bind(struct sllp_server* server, struct sllp_address *addr);
struct sllp_server* sllp_create_tcpserver(struct sllp_address* addr);
void sllp_free_tcpserver(struct sllp_server* server);
struct sllp_server* sllp_create_udpserver(struct sllp_address* addr);
void sllp_free_udpserver(struct sllp_server* server);

int32_t dummy__handle_error(struct sllp_server* server, struct sllp_socket* request);
int32_t dummy__handle_timeout(struct sllp_server* server);
int32_t dummy__verify_request(struct sllp_server* server, struct sllp_socket* request);
int32_t dummy__server_close(struct sllp_server* server);
int32_t dummy__process_request(struct sllp_server* server, struct sllp_socket* request);

struct sllp_socket* tcp__get_request(struct sllp_server* server);
int32_t tcp__server_activate(struct sllp_server* server);
int32_t tcp__close_request(struct sllp_server* server, struct sllp_socket* request);

/* end of function declare */

struct sllp_server* internal_create_server(int32_t type)
{
    struct sllp_server* server;

    server = malloc(sizeof(struct sllp_server));
    if (!server)
	return NULL;

    server->type = type;
    server->allow_reuse_address = 1;
    server->timeout = 1;

    server->server_forever = server_server_forever;
    server->shutdown = server_shutdown;
    server->handle_request = server_handle_request;
    server->__server_bind = server__server_bind;
    server->__handle_request_noblock = server__handle_request_noblock;

    server->__server_close = dummy__server_close;
    server->__handle_error = dummy__handle_error;
    server->__process_request = dummy__process_request;
    server->__handle_timeout = dummy__handle_timeout;
    server->__verify_request = dummy__verify_request;

    if (server->type == MAKE_WORD3('U','D','P'))
    {
	server->sock = sllp_create_socket(AF_INET, SOCK_DGRAM, 0);
	server->__server_activate = NULL;
	server->__close_request = NULL;
	server->__get_request = NULL;
    }
    else if (server->type == MAKE_WORD3('T','C','P'))
    {
	server->sock = sllp_create_socket(AF_INET, SOCK_STREAM, 0);
	server->request_queue_size = 5;
	server->__server_activate = tcp__server_activate;
	server->__close_request = tcp__close_request;
	server->__get_request = tcp__get_request;
    }

    return server;
}

void internal_free_server(struct sllp_server* server)
{
    server->__server_close(server);
    sllp_free_socket(server->sock);
    free(server);
}

/* Public function:
 *	*int32_t server_server_forever(struct sllp_server *server, int32_t poll_interval);
 *	*int32_t server_shutdown(struct sllp_server* server);
 *	*int32_t server_handle_request(struct sllp_server* server);
 *
 */

int32_t server_server_forever(struct sllp_server *server, int32_t poll_interval)
{
    struct sllp_socket_list *rlist, *outlist;
    struct sllp_select_result result;

    server->__serving = 1;

    rlist = sllp_create_socket_list();
    outlist = sllp_create_socket_list();

    rlist->append(rlist, server->sock);
    result.rlist = outlist;
    result.wlist = NULL;
    result.elist = NULL;

    while (server->__serving)
    {
	sllp_select(rlist, NULL, NULL, poll_interval, &result);
	if (result.rlist->count)
	{
	    server->__handle_request_noblock(server);
	}
    }

    sllp_free_socket_list(rlist, OPT_ON);
    sllp_free_socket_list(outlist, OPT_OFF);

    return 0;
}

int32_t server_shutdown(struct sllp_server* server)
{
    server->__serving = 0;
    return 0;
}

int32_t server_handle_request(struct sllp_server* server)
{
    struct sllp_socket_list *rlist, *outlist;
    struct sllp_select_result result;
    int32_t n;

    rlist = sllp_create_socket_list();
    outlist = sllp_create_socket_list();

    rlist->append(rlist, server->sock);
    n = sllp_select(rlist, NULL, NULL, server->timeout, &result);
    if (!n)
	server->__handle_timeout(server);
    server->__handle_request_noblock(server);
	
    sllp_free_socket_list(rlist, OPT_ON);
    sllp_free_socket_list(outlist, OPT_OFF);
    
    return 0;
}

/*
 * The following is the private function
 */

int32_t server__handle_request_noblock(struct sllp_server *server)
{
    struct sllp_socket *request;

    request = server->__get_request(server);
    if (server->__verify_request(server, request) < 0)
	return -1;

    if (server->__process_request(server, request) < 0)
    {
	server->__handle_error(server, request);
	server->__close_request(server, request);
	return -1;
    }

    return 0;
}

int32_t server__server_bind(struct sllp_server* server, struct sllp_address *addr)
{
    int32_t ret;

    ret = 0;

    if (server->allow_reuse_address)
	ret = server->sock->setsockopt(server->sock, SOL_SOCKET, SO_REUSEADDR, (void*)&OPT_ON, sizeof(OPT_ON));
    if (ret < 0)
	return -1;

    ret = server->sock->bind(server->sock, addr);
    if (ret < 0)
	return -1;
    ret = server->sock->getsockname(server->sock, &server->addr);
    if (ret < 0)
	return -1;
    return 0;
}

struct sllp_server* sllp_create_tcpserver(struct sllp_address* addr)
{
    struct sllp_server *server;

    server = internal_create_server(MAKE_WORD3('T','C','P'));
    assert(server);
    server->__server_bind(server, addr);
    server->__server_activate(server);
    return server;
}

void sllp_free_tcpserver(struct sllp_server* server)
{
    internal_free_server(server);
}

struct sllp_server* sllp_create_udpserver(struct sllp_address* addr)
{
    struct sllp_server *server;

    server = internal_create_server(MAKE_WORD3('T','C','P'));
    assert(server);
    server->__server_bind(server, addr);
    return server;
}

void sllp_free_udpserver(struct sllp_server* server)
{
    internal_free_server(server);
}


/*
 * implementation by subclass
 */

int32_t dummy__handle_error(struct sllp_server* server, struct sllp_socket* request)
{
    /* Noting todo */
    return 0;
}

int32_t dummy__handle_timeout(struct sllp_server* server)
{
    /* Noting todo */
    return 0;
}

int32_t dummy__verify_request(struct sllp_server* server, struct sllp_socket* request)
{
    /* Noting todo */
    return 0;
}

int32_t dummy__server_close(struct sllp_server* server)
{
    /* Noting todo */
    return 0;
}

int32_t dummy__process_request(struct sllp_server* server, struct sllp_socket* request)
{
    /* Noting todo */
   
    u_int8_t buf[1024];
    u_int32_t datalen;
    /* Test coding */
    memset(buf, 0, sizeof(buf));
    datalen = request->recv(request, buf, sizeof(buf), 0);
    if (!datalen)
	server->__close_request(server, request);
    printf("receive data from: (%s, %d), len: %d, data: %s\n", request->address.host, request->address.port, datalen, buf);
    request->sendall(request, buf, datalen, 0);
    return 0;
}


/*
 * TCP
 */
struct sllp_socket* tcp__get_request(struct sllp_server* server)
{
    return server->sock->accept(server->sock);
}

int32_t tcp__server_activate(struct sllp_server* server)
{
    server->sock->listen(server->sock, server->request_queue_size);
    return 0;
}

int32_t tcp__close_request(struct sllp_server* server, struct sllp_socket* request)
{
    return request->close(request);
}

