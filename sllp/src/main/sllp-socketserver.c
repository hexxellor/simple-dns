#include "sllp-socketserver.h"
#include "sllp-select.h"
#include "sllp-algo.h"

/* function declare */
static struct sllp_request* create_request(struct sllp_server*server);
static void free_request(u_long* request);

struct sllp_server* internal_create_server(int32_t type);
void internal_free_server(struct sllp_server* server);
int32_t server_server_forever(struct sllp_server *server, int32_t poll_interval);
int32_t server_shutdown(struct sllp_server* server);
int32_t server_handle_request(struct sllp_server* server);

int32_t server__handle_request_noblock(struct sllp_server *server);
int32_t server__server_bind(struct sllp_server* server, struct sllp_address *addr);
int32_t server__server_close(struct sllp_server* server);;

struct sllp_server* sllp_create_tcpserver(struct sllp_address* addr);
void sllp_free_tcpserver(struct sllp_server* server);
struct sllp_server* sllp_create_udpserver(struct sllp_address* addr);
void sllp_free_udpserver(struct sllp_server* server);

struct sllp_request* tcp__get_request(struct sllp_server* server);
int32_t tcp__server_activate(struct sllp_server* server);
int32_t tcp__close_request(struct sllp_server* server, struct sllp_request* request);

struct sllp_request* udp__get_request(struct sllp_server* server);
int32_t udp__server_activate(struct sllp_server* server);
int32_t udp__close_request(struct sllp_server* server, struct sllp_request* request);

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
    server->__server_close = server__server_close;

    server->__handle_error = NULL;
    server->__process_request = NULL;
    server->__handle_timeout = NULL;
    server->__verify_request = NULL;

    if (server->type == MAKE_WORD3('U','D','P'))
    {
	server->sock = sllp_create_socket(AF_INET, SOCK_DGRAM, 0);
	server->__server_activate = udp__server_activate;
	server->__close_request = udp__close_request;
	server->__get_request = udp__get_request;

	server->__udp_request = create_request(server);
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

    sllp_free_socket_list(rlist, ON);
    sllp_free_socket_list(outlist, OFF);

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
	
    sllp_free_socket_list(rlist, ON);
    sllp_free_socket_list(outlist, OFF);
    
    return 0;
}

/*
 * The following is the private function
 */
/* 
 * Process Short Community.
 */
int32_t server__handle_request_noblock(struct sllp_server *server)
{
    struct sllp_request *request;
    int32_t ret;

    /* TODO: we need a request manager;
     *	     How to deal the UDP ?
     */
    request = server->__get_request(server);
    if (server->__verify_request(server, request) < 0)
    {
	server->__close_request(server, request);
	return -1;
    }

    ret = server->__process_request(server, request);
    if (ret == -1)
    {	/* remote client close the connect */
	server->__close_request(server, request);
	return -1;
    }
    else if (ret == -2)
    {	/* occur an error */
	server->__handle_error(server, request);
    }
    
    server->__close_request(server, request);

    return 0;
}

int32_t server__server_bind(struct sllp_server* server, struct sllp_address *addr)
{
    int32_t ret;

    ret = 0;

    if (server->allow_reuse_address)
	ret = server->sock->setsockopt(server->sock, SOL_SOCKET, SO_REUSEADDR, (void*)&ON, sizeof(ON));
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

int32_t server__server_close(struct sllp_server* server)
{
    /* close the server socket */
    server->__server_close(server);
    sllp_free_socket(server->sock);

    /* TODO: record on the log */
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


static struct sllp_request* create_request(struct sllp_server*server)
{
    struct sllp_request* request;

    request = malloc(sizeof(struct sllp_request));

    if (server->type == MAKE_WORD3('U','D','P'))
	request->sock = server->sock;
    else
	request->sock = NULL;

    request->bufsize = SLLP_MAX_PACKETSIZE;
    request->recvlen = 0;

    return request;
}

static void free_request(u_long* ptr)
{
    struct sllp_request* request;

    request = (struct sllp_request*)ptr;
    sllp_free_socket(request->sock);
    free(request);
}

/*
 * TCP
 */
struct sllp_request* tcp__get_request(struct sllp_server* server)
{
    struct sllp_request* request;

    request = create_request(server);
    request->sock = server->sock->accept(server->sock);
    return request;
}

int32_t tcp__server_activate(struct sllp_server* server)
{
    server->sock->listen(server->sock, server->request_queue_size);
    return 0;
}

int32_t tcp__close_request(struct sllp_server* server, struct sllp_request* request)
{
    free_request((u_long*)request);
    return 0;
}

/*
 * UDP
 */
struct sllp_request* udp__get_request(struct sllp_server* server)
{
    struct sllp_request* request;
    struct sllp_socket* sock;
    int32_t len;

    request = server->__udp_request;
    sock = request->sock;

    memset(request->data, 0, request->bufsize);
    len = sock->recvfrom(sock, request->data, request->bufsize, 0, &request->sock->address);
    request->recvlen = len;
    return request;
}

int32_t udp__server_activate(struct sllp_server* server)
{
    /* nothing todo for udp */
    return 0;
}

int32_t udp__close_request(struct sllp_server* server, struct sllp_request* request)
{
    return 0;
}

