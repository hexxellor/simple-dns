/* Define common included header of SLLP.

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

#ifndef __SLLP_GLOBAL_H_
#define __SLLP_GLOBAL_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 
#include <sys/un.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

/* type */
typedef int SOCKET_T;

/* service define */
#define SLLP_SERV_DATE	1
#define SLLP_SERV_FTP	2

/* service port define */
#define SLLP_SERV_FTP_PORT	21

/* limit */
#define SLLP_ADDRESS_LEN	32

#define SOCKETCLOSE(x)	\
    close(x)

#define	SAS2SA(x) 	\
    ((struct sockaddr *)(x))

#define MAKE_WORD3(w0, w1, w2)		\
    MAKE_WORD4(w0, w1, w2, 0)

#define MAKE_WORD4(w0, w1, w2, w3)	\
    ((w0 << 24) | (w1 << 16) | (w2 << 8) | w3)

#endif
