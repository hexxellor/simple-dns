#ifndef __SLLP_SOCKETSERVER_H_
#define __SLLP_SOCKETSERVER_H_

#include "sllp-global.h"
#include "sllp-socket.h"

struct sllp_server;
struct sllp_request;

/* TODO: we need change the element of __requst_list from sllp_socket* to
 * 	 sllp_request*.
 *
 */
#define SERVER_HEADER				\
    int32_t type;				\
    int32_t timeout;				\
    int32_t allow_reuse_address;		\
    struct sllp_socket* sock;			\
    int32_t request_queue_size;			\
    struct sllp_address	addr;			\
    int32_t __serving;				\
    struct sllp_socket_list *__requst_list


#define SERVER_PUBLIC_METHOD						\
    int32_t (*server_forever)(struct sllp_server*, int32_t poll_interval); \
    int32_t (*shutdown)(struct sllp_server*);				\
    int32_t (*handle_request)(struct sllp_server*)

#define SERVER_PRIVATE_METHOD						\
    int32_t (*__server_bind)	(struct sllp_server*, struct sllp_address* addr);   \
    int32_t (*__server_activate)(struct sllp_server*);			\
    int32_t (*__handle_timeout)	(struct sllp_server*);			\
    int32_t (*__verify_request)	(struct sllp_server*, struct sllp_request* request); \
    int32_t (*__server_close)	(struct sllp_server*);			\
    int32_t (*__process_request)(struct sllp_server*, struct sllp_request* request); \
    int32_t (*__close_request)	(struct sllp_server*, struct sllp_request* request); \
    int32_t (*__handle_error)	(struct sllp_server*, struct sllp_request* request); \
    int32_t (*__handle_request_noblock)(struct sllp_server*);		\
    int32_t (*__finish_request)(struct sllp_server*, struct sllp_request* request); \
    struct sllp_socket* (*__get_request)(struct sllp_server*)

struct sllp_server{
    SERVER_HEADER;
    SERVER_PUBLIC_METHOD;
    SERVER_PRIVATE_METHOD;
};
struct sllp_server* sllp_create_tcpserver(struct sllp_address* addr);
void sllp_free_tcpserver(struct sllp_server* server);

struct sllp_server* sllp_create_udpserver(struct sllp_address* addr);
void sllp_free_udpserver(struct sllp_server* server);

struct sllp_request{
    struct sllp_socket* sock;
    u_int8_t 		data[MAX_PACKETSIZE];
    u_int16_t		len;
};

#endif
