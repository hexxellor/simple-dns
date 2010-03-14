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
#include "sllp-limit.h"

struct sllp_socket;
struct sllp_address;

struct sllp_address{
    char host[SLLP_ADDRESS_LEN];
    u_int16_t port;
};
/**
 * assign the 'host' and 'port' to the 'address.
 */
u_int32_t sllp_make_address(char* host, u_int16_t port, struct sllp_address *address);

/**
 * Making a connection to the address. If you assign the timeout, it will continous making
 * the connection until either occur an error or timeout.
 */
u_int32_t sllp_create_connection(struct sllp_address *address, u_int32_t timeout);

/**
 * Get the ip address of hostname.
 */
u_int32_t sllp_gethostbyname(char* hostname);

/**
 * Get the hostname by ip address.
 */
u_int32_t sllp_gethostbyaddr(char* ipaddress);

/**
 * Get the local machine of the process running.
 */
u_int32_t sllp_gethostname();

/**
 * Get the sllp_address from socket object.
 */
u_int32_t sllp_getnameinfo(struct sllp_socket *socket);

/**
 * Get the constal value of protocol from a string that describe the protocol.
 * For example: Passing "UDP" will return SOCK_DAGRAM.
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
 * Create a new sllp_socket object, by specifics the argument.
 */
u_int32_t sllp_socket(u_int8_t family, u_int8_t type, u_int8_t protocol, struct sllp_socket* socket);

/**
 * Create a socket pairt. This function is only aviable on UNIX plantfrom, is using the Unix-Domain socket.
 */
u_int32_t sllp_socketpair(u_int8_t family, u_int8_t type, u_int8_t protocol, struct sllp_socket_pair* socketpair);

/**
 * Attach the fd to a new socket object.
 */
u_int32_t sllp_fromfd(u_int32_t fd, u_int8_t family, u_int8_t type, u_int8_t protocol, struct sllp_socket* socket);

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
u_int32_t sllp_inet_ntoa(u_int32_t ip, char* ip_string, u_int32_t size);

/**
 * Get the default timeout of the sllp.
 */
u_int32_t sllp_getdefaulttimeout();

/**
 * Set the default timeout of the sllp.
 */
u_int32_t sllp_setdefaulttimeout(u_int32_t timeout);

struct sllp_socket{
    u_int32_t (*accept)		(struct sllp_socket* this, struct sllp_socket*, struct sllp_address*);
    u_int32_t (*bind)    	(struct sllp_socket* this, struct sllp_address);
    u_int32_t (*close)		(struct sllp_socket* this);
    u_int32_t (*connection)	(struct sllp_socket* this, struct sllp_address*);
    u_int32_t (*fileno)		(struct sllp_socket* this);
    u_int32_t (*getpeername)	(struct sllp_socket* this, struct sllp_address*);
    u_int32_t (*getsockname)	(struct sllp_socket* this, struct sllp_address*);
    u_int32_t (*getsocketopt)	(struct sllp_socket* this, u_int32_t level, u_int32_t optname);
    u_int32_t (*listen)		(struct sllp_socket* this, u_int32_t backlog);
    u_int32_t (*recv)		(struct sllp_socket* this, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag);
    u_int32_t (*recvfrom)	(struct sllp_socket* this, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag);
    u_int32_t (*send)		(struct sllp_socket* this, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag);
    u_int32_t (*sendall)	(struct sllp_socket* this, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag);
    u_int32_t (*setblocking)	(struct sllp_socket* this, u_int8_t true_or_false);
    u_int32_t (*settimeout)	(struct sllp_socket* this, u_int32_t value);
    u_int32_t (*gettimeout)	(struct sllp_socket* this);
    u_int32_t (*setsocketopt)	(struct sllp_socket* this, u_int32_t level, u_int32_t optname, u_int32_t value);
    u_int32_t (*shutdown)	(struct sllp_socket* this, u_int32_t how);
};
u_int32_t sllp_init_socket(struct sllp_socket* socket);

#endif
