/* Define wrapper of BSD-socket interface.

   Copyright (C) 2010 tengjiaozhao@gmail.com
   This file is part of the SLLP Library.

   The SLLP Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The SLLP Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
*/


#ifndef __SLLP_SOCKET_H_
#define __SLLP_SOCKET_H_

#include "sllp-global.h"

struct sllp_socket;
struct sllp_address;

struct sllp_address{
    char host[SLLP_ADDRESS_LEN];
    u_int16_t port;
};

struct sllp_socket_pair{
    struct sllp_socket* s0;
    struct sllp_socket*	s1;
};
/**
 * assign the 'host' and 'port' to the 'address.
 */
u_int32_t sllp_make_address(char* host, u_int16_t port, struct sllp_address *address);

/**
 * Get the ip address of hostname. Packet the ipaddress into a 32-bit value.
 */
u_int32_t sllp_gethostbyname(char* hostname);

/**
 * Get the local machine of the process running.
 */
u_int32_t sllp_gethostname(char* hostname, u_int32_t buf_len);

/**
 * Get the constal value of protocol from a string that describe the protocol.
 * For example: Passing "UDP" will return SOCK_DGRAM.
 */
u_int32_t sllp_getprotobyname(char *protocolname);

/**
 * Get the constal value of internet service from a string that describe the protocol.
 * For example: Passing "ftp", "UDP" will return SLLP_SERVICE_FTP
 */
u_int32_t sllp_getservbyname(char* servicename, char* protocolname);

/**
 * The same as sllp_getservbyname, but not using servicename, it uses service port.
 */
u_int32_t sllp_getservbyport(u_int16_t port, char* protocolname);

/**
 * Create a socket pairt. This function is only aviable on UNIX plantfrom, is using the Unix-Domain socket.
 */
struct sllp_socket_pair* sllp_socketpair(u_int8_t family, u_int8_t type, u_int8_t protocol);

/**
 * Attach the fd to a new socket object.
 */
struct sllp_socket* sllp_fromfd(u_int32_t fd, u_int8_t family, u_int8_t type, u_int8_t protocol);

/**
 * Convert 32-bit positive integer from network to host byte order.
 */
u_int32_t sllp_ntohl(u_int32_t x);

/**
 * Convert 16-bit positive integer from network to host byte order.
 */
u_int16_t sllp_ntohs(u_int16_t x);

/**
 * Convert 32-bit positive integer from host to network byte order.
 */
u_int32_t sllp_htonl(u_int32_t x);

/**
 * Convert 16-bit positive integer from host to network byte order.
 */
u_int16_t sllp_htons(u_int16_t x);


/**
 * Convert the ip string to the 32-bit packet binary.
 */
u_int32_t sllp_inet_aton(char *ip_string);

/**
 * Convert the 32-bit packet binary address to the ip string.
 */
char* sllp_inet_ntoa(u_int32_t ip);

/**
 * Get the default timeout of the sllp.
 */
u_int32_t sllp_getdefaulttimeout();

/**
 * Set the default timeout of the sllp.
 */
void sllp_setdefaulttimeout(u_int32_t timeout);

/**
 * Crate socket object using the specifies argments and init the socket object.
 */
struct sllp_socket* sllp_create_socket(u_int8_t family, u_int8_t type, u_int8_t protocol);

/**
 * Free socket object
 */
u_int32_t sllp_free_socket(struct sllp_socket * sock);

struct sllp_socket{
    struct sllp_socket*(*accept)(struct sllp_socket*);
    int32_t (*bind)		(struct sllp_socket*, struct sllp_address*);
    int32_t (*close)		(struct sllp_socket*);
    int32_t (*connect)		(struct sllp_socket*, struct sllp_address*);
    int32_t (*fileno)		(struct sllp_socket*);
    int32_t (*getpeername)	(struct sllp_socket*, struct sllp_address*);
    int32_t (*getsockname)	(struct sllp_socket*, struct sllp_address*);
    int32_t (*getsocketopt)	(struct sllp_socket*, u_int32_t level, u_int32_t optname, void* buf, socklen_t *buf_len);
    int32_t (*listen)		(struct sllp_socket*, u_int32_t backlog);
    int32_t (*recv)		(struct sllp_socket*, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag);
    int32_t (*recvfrom)		(struct sllp_socket*, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag, struct sllp_address*);
    int32_t (*send)		(struct sllp_socket*, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag);
    int32_t (*sendall)		(struct sllp_socket*, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag);
    int32_t (*sendto)		(struct sllp_socket*, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag, struct sllp_address*);
    int32_t (*setblocking)	(struct sllp_socket*, u_int8_t block);
    void    (*settimeout)	(struct sllp_socket*, u_int32_t value);
    int32_t (*gettimeout)	(struct sllp_socket*);
    int32_t (*setsockopt)	(struct sllp_socket*, u_int32_t level, u_int32_t optname, void* value, int32_t len);
    int32_t (*shutdown)		(struct sllp_socket*, u_int32_t how);
    
    struct sllp_address		address;	
    SOCKET_T			fd;
    u_int32_t			family;
    u_int32_t			type;
    u_int32_t			proto;
    int32_t			timeout;
};

extern int32_t	OPT_ON;
extern int32_t	OPT_OFF;

#endif
