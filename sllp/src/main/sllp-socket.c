#include "sllp-socket.h"

/* function declare */
static struct sllp_socket_pair* new_sllp_socket_pair(struct sllp_socket* s0, struct sllp_socket* s1);
static struct sllp_socket* new_sllp_socket(SOCKET_T fd, u_int32_t family, u_int32_t type, u_int32_t proto);
static void init_sllp_socket(struct sllp_socket *sock, SOCKET_T fd, u_int32_t family, u_int32_t type, u_int32_t proto);
static int32_t sllp_getsocketaddrlen(struct sllp_socket* sock);
static int32_t internal_select(struct sllp_socket* sock, int32_t writing);
static int32_t sllp_address2sas(struct sllp_address* addr, struct sockaddr_in* sas);
static int32_t sllp_sas2address(struct sockaddr_in* sas, struct sllp_address* addr);
static int32_t internal_connect(struct sllp_socket* sock, struct sockaddr* addr, int32_t addr_len, int32_t *timeoutp);
static int32_t sock_recv_guts(struct sllp_socket* sock, u_int8_t* cbuf, int32_t len, int32_t flags);
static int32_t sock_recvfrom_guts(struct sllp_socket *sock, u_int8_t* cbuf, int32_t len, int32_t flags, struct sllp_address *addr);
static int32_t internal_setblocking(struct sllp_socket* sock, u_int32_t block);

struct sllp_socket* sock_accept(struct sllp_socket* sock);
int32_t sock_bind(struct sllp_socket* sock, struct sllp_address* address);
int32_t sock_close(struct sllp_socket* sock);
int32_t sock_connect(struct sllp_socket* sock, struct sllp_address* addr);
int32_t sock_fileno(struct sllp_socket* sock);
int32_t sock_getpeername(struct sllp_socket* sock, struct sllp_address* addr);
int32_t sock_getsockname(struct sllp_socket* sock, struct sllp_address* addr);
int32_t sock_getsocketopt(struct sllp_socket* sock, u_int32_t level, u_int32_t optname, void* buf, socklen_t* buf_len);
int32_t sock_listen(struct sllp_socket* sock, u_int32_t backlog);
int32_t sock_recv(struct sllp_socket* sock, u_int8_t *buf, u_int32_t bufsize, u_int32_t flags);
int32_t sock_recvfrom(struct sllp_socket* sock, u_int8_t *buf, u_int32_t bufsize, u_int32_t flags, struct sllp_address *addr);
int32_t sock_send(struct sllp_socket* sock, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag);
int32_t sock_sendall(struct sllp_socket* sock, u_int8_t *buf, u_int32_t len, u_int32_t flag);
int32_t sock_sendto(struct sllp_socket* sock, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag, struct sllp_address* addr);
int32_t sock_setblocking(struct sllp_socket* sock, u_int8_t block);
void sock_settimeout(struct sllp_socket* sock, u_int32_t value);
int32_t sock_gettimeout(struct sllp_socket* sock);
int32_t sock_setsockopt(struct sllp_socket* sock, u_int32_t level, u_int32_t optname, void* value, int32_t len);
int32_t sock_shutdown(struct sllp_socket* sock, u_int32_t how);
/* end function declare */

static u_int32_t defaulttimeout = -1;
int32_t	OPT_ON 	= 1;
int32_t	OPT_OFF = 0;

static struct sllp_socket_pair* new_sllp_socket_pair(struct sllp_socket* s0, struct sllp_socket* s1)
{
    struct sllp_socket_pair* pair = NULL;
    pair = malloc(sizeof(struct sllp_socket_pair));
    if (!pair)
    {
	errno = ENOMEM;
	return NULL;
    }
    pair->s0 = s0;
    pair->s1 = s1;
    return pair;
}

static struct sllp_socket* new_sllp_socket(SOCKET_T fd, u_int32_t family, u_int32_t type, u_int32_t proto)
{
    struct sllp_socket* sock;
    sock = malloc(sizeof(struct sllp_socket));
    if (!sock)
    {
	errno = ENOMEM;
	return NULL;
    }
    init_sllp_socket(sock, fd, family, type, proto);
    return sock;
}

static void init_sllp_socket(struct sllp_socket *sock, SOCKET_T fd, u_int32_t family, u_int32_t type, u_int32_t proto)
{
    /* TODOList: 
     *	set the method of socket object
     */
    if(!sock)
	return;

    /* init variables */
    sock->fd = fd;
    sock->family = family;
    sock->type = type;
    sock->proto = proto;

    /* init methods */
    sock->accept = sock_accept;
    sock->bind	 = sock_bind;
    sock->close	 = sock_close;
    sock->connect= sock_connect;
    sock->fileno = sock_fileno;
    sock->getpeername = sock_getpeername;
    sock->getsockname = sock_getsockname;
    sock->getsocketopt= sock_getsocketopt;
    sock->listen      = sock_listen;
    sock->recv	      = sock_recv;
    sock->recvfrom    = sock_recvfrom;
    sock->send	      = sock_send;
    sock->sendall     = sock_sendall;
    sock->sendto      = sock_sendto;
    sock->setblocking = sock_setblocking;
    sock->settimeout  = sock_settimeout;
    sock->gettimeout  = sock_gettimeout;
    sock->setsockopt= sock_setsockopt;
    sock->shutdown    = sock_shutdown;
}

static int32_t sllp_getsocketaddrlen(struct sllp_socket* sock)
{
    if (!sock)
    {
	return -1;
    }
    
    int32_t addr_len = 0;
    switch (sock->family)
    {
	case AF_UNIX:
	    addr_len = sizeof(struct sockaddr_un);
	    break;
	case AF_INET:
	    addr_len = sizeof(struct sockaddr_in);
	    break;
	case AF_INET6:
	    addr_len = sizeof(struct sockaddr_in6);
	    break;
	case AF_PACKET:
	    addr_len = sizeof(struct sockaddr_ll);
	    break;
	default:
	    addr_len = -1;
	    break;
    }
    return addr_len;
}

static int32_t internal_select(struct sllp_socket* sock, int32_t writing)
{
    /* This function works in timeout mode 
       If the socket is in blocking mode, we need not use select to wait.
       If the socket is in non-blocking mode, but has not timeout value,
       which means the socket need return immeditly, also need not wait.
     */
    if (sock->timeout <= 0)	
	return 0;

    /* invalid socke file descriptor */
    if (sock->fd < 0)
	return 0;

    /* make select */
    int32_t n;
    fd_set fds;
    struct timeval tv;
    tv.tv_sec = (int32_t)sock->timeout;
    tv.tv_usec = (int)((sock->timeout - tv.tv_sec) * 1e6);
    FD_ZERO(&fds);
    FD_SET(sock->fd, &fds);

    if (writing)
	n = select(sock->fd + 1, NULL, &fds, NULL, &tv);
    else
	n = select(sock->fd + 1, &fds, NULL, NULL, &tv);

    if (n < 0)
	return -1;
    if (n == 0)
	return 1;
    return 0;
}

static int32_t sllp_address2sas(struct sllp_address* addr, struct sockaddr_in* sas)
{
    if (!addr || !sas)
	return -1;
    /* TODOList: need to support other protocol */
    sas->sin_family = AF_INET;
    sas->sin_port = htons(addr->port);
    sas->sin_addr.s_addr = sllp_inet_aton(addr->host);
    return 0;
}

static int32_t sllp_sas2address(struct sockaddr_in* sas, struct sllp_address* addr)
{
    if (!sas || !addr)
	return -1;

    char* ip;
    ip = inet_ntoa(sas->sin_addr);
    strcpy(addr->host, ip);
    addr->port = ntohs(sas->sin_port);
    return 0;
}

static int32_t internal_connect(struct sllp_socket* sock, struct sockaddr* addr, int32_t addr_len, int32_t *timeoutp)
{
    int32_t res, timeout;
    if (!sock || !addr || !timeoutp)
	return -1;

    timeout = 0;
    res = connect(sock->fd, addr, addr_len);

    if (sock->timeout > 0)
    {
	if (res < 0 && errno == EINPROGRESS)
	{
	    /* waiting to write */
	    timeout = internal_select(sock, 1);
	    if (timeout == 0)
	    {
		/* get the real errno reason */
		u_int32_t res_size = sizeof(res);
		getsockopt(sock->fd, SOL_SOCKET, SO_ERROR, &res, &res_size);
		if (res == EISCONN)
		    res = 0;
		errno = res;
	    }
	    else if (timeout == -1)
	    {
		/* handle error */
		res = errno;
	    }
	    else
		res = EWOULDBLOCK;	/* time out */
	}
    }

    if (res < 0)
	res = errno;

    *timeoutp = timeout;
    return res;
}

static int32_t sock_recv_guts(struct sllp_socket* sock, u_int8_t* cbuf, int32_t len, int32_t flags)
{
    int32_t outlen = -1;
    int32_t timeout;

    timeout = internal_select(sock, 0);
    if (!timeout) /* ok */
	outlen = recv(sock->fd, cbuf, len, flags);

    if (timeout == 1) /* time out */
	return -1;	

    if (outlen < 0)
	return -1;

    return outlen;
}

static int32_t sock_recvfrom_guts(struct sllp_socket *sock, 
				  u_int8_t* cbuf, 
				  int32_t len, 
				  int32_t flags, 
				  struct sllp_address *addr)
{
    struct sockaddr_in addrbuf;
    int32_t timeout, outlen;
    socklen_t addr_len;

    addr_len = sllp_getsocketaddrlen(sock);
    
    timeout = internal_select(sock, 0);
    if (!timeout)
	outlen = recvfrom(sock->fd, cbuf, len, flags, SAS2SA(&addrbuf), &addr_len);

    if (timeout == 1)
	return -1;	/* timeout */

    if (outlen < 0)
	return -1;

    sllp_sas2address(&addrbuf, addr);
    return outlen;
}

static int32_t internal_setblocking(struct sllp_socket* sock, u_int32_t block)
{
    int32_t flags;

    flags = fcntl(sock->fd, F_GETFL, 0);
    if (flags == -1)
	return -1;
    return fcntl(sock->fd, F_SETFL, flags|O_NONBLOCK);
}

/***************************************************
 *
 * The following is the module function, and sock object method.
 * sllp_make_address(host, port, address) -> result;
 *
 *
 *
 *
 *
 *
 *
 ****************************************************/

u_int32_t sllp_make_address(char* host, u_int16_t port, struct sllp_address *address)
{
    if (!address)
	return -1;
    strcpy(address->host, host);
    address->port = port;
    return 0;
}

u_int32_t sllp_gethostbyname(char* hostname)
{
    u_int32_t ip;
    struct hostent *he = gethostbyname(hostname);
    if (he)
    {
	memcpy(&ip, he->h_addr_list[0], 4);
	return ip;
    }
    return -1;
}

u_int32_t sllp_gethostname(char* hostname, u_int32_t buf_len)
{
    return gethostname(hostname, buf_len);	
}

u_int32_t sllp_getprotobyname(char *protocolname)
{
    if (!strcmp("UDP", protocolname))
	return SOCK_DGRAM;
    else if (!strcmp("TCP", protocolname))
	return SOCK_STREAM;
    else if (!strcmp("RAW", protocolname))
	return SOCK_RAW;
    return -1;
}

u_int32_t sllp_getservbyname(char* servicename, char* protocolname)
{
    if (!strcmp("FTP", servicename) && !strcmp("TCP", protocolname))
	return SLLP_SERV_FTP;
    else if ((!strcmp("DATE", servicename) && !strcmp("TCP", protocolname)) ||
	     (!strcmp("DATE", servicename) && !strcmp("UDP", protocolname))
	)
	return SLLP_SERV_DATE;
    return -1;
}

u_int32_t sllp_getservbyport(u_int16_t port, char* protocolname)
{
    return -1;
}

struct sllp_socket_pair* sllp_socketpair(u_int8_t family, u_int8_t type, u_int8_t protocol)
{
    SOCKET_T sv[2];
    struct sllp_socket *s0 = NULL, *s1 = NULL;
    struct sllp_socket_pair *res = NULL;

    if (socketpair(family, type, protocol, sv) < 0)
    {
	goto finally;
    }
    s0 = sllp_fromfd(sv[0], family, type, protocol);
    if (!s0)
	goto finally;
    s1 = sllp_fromfd(sv[1], family, type, protocol);
    if (!s1)
	goto finally;
    
    res = new_sllp_socket_pair(s0, s1);
    
  finally:
    if (!res)
    {
	if (s0)
	{
	    SOCKETCLOSE(sv[0]);
	    sllp_free_socket(s0);
	}
	if (s1)
	{
	    SOCKETCLOSE(sv[1]);
	    sllp_free_socket(s1);
	}
    }
    return res;
}

struct sllp_socket* sllp_fromfd(u_int32_t fd, u_int8_t family, u_int8_t type, u_int8_t protocol)
{
    struct sllp_socket* s = NULL;
    s = new_sllp_socket(fd, family, type, protocol);
    return s;
}

u_int32_t sllp_ntohl(u_int32_t x)
{
    return ntohl(x);
}

u_int16_t sllp_ntohs(u_int16_t x)
{
    return ntohs(x);
}

u_int32_t sllp_htonl(u_int32_t x)
{
    return htonl(x);
}

u_int16_t sllp_htons(u_int16_t x)
{
    return htons(x);
}

u_int32_t sllp_inet_aton(char *ip_string)
{
    u_int32_t packed_addr;
    
    if (!strcmp(ip_string, "0.0.0.0"))
	packed_addr = INADDR_ANY;
    else
	packed_addr = inet_addr(ip_string);
    return packed_addr;
}

char* sllp_inet_ntoa(u_int32_t ip)
{
    struct in_addr packed_addr;
    packed_addr.s_addr = ip;
    return inet_ntoa(packed_addr);
}

u_int32_t sllp_getdefaulttimeout()
{
    return defaulttimeout;
}

void sllp_setdefaulttimeout(u_int32_t timeout)
{
    defaulttimeout = timeout;
}

/* s.accpet() method */
struct sllp_socket* sock_accept(struct sllp_socket* sock)
{
    struct sockaddr_in addrbuf;
    SOCKET_T newfd;
    int32_t timeout;
    socklen_t addr_len;
    struct sllp_socket* newsock;

    if (!sock)
    {
	return NULL;
    }

    addr_len = sllp_getsocketaddrlen(sock);
    
    memset(&addrbuf, 0, addr_len);
    timeout = internal_select(sock, 0);
    if (!timeout)
	newfd = accept(sock->fd, SAS2SA(&addrbuf), &addr_len);	

    if (timeout == 1)
    {
	return NULL;
    }

    if (newfd < 0)
	return NULL;
    
    newsock = sllp_fromfd(newfd, sock->family, sock->type, sock->proto);
    
    char* ip = sllp_inet_ntoa(addrbuf.sin_addr.s_addr);
    sllp_make_address(ip, addrbuf.sin_port, &newsock->address);

    return newsock;
}

/* s.bind() */
int32_t sock_bind(struct sllp_socket* sock, struct sllp_address* address)
{
    int32_t res;
    struct sockaddr_in addrbuf;
    socklen_t addrlen;

    if (!sock)
	return -1;
    
    if (sllp_address2sas(address, &addrbuf) < 0)
	return -1;

    addrlen = sllp_getsocketaddrlen(sock);

    res = bind(sock->fd, SAS2SA(&addrbuf), addrlen);
    return res;
}

int32_t sock_close(struct sllp_socket* sock)
{
    if (!sock)
	return -1;
    
    if (sock->fd != -1)
    {
	SOCKETCLOSE(sock->fd);
	sock->fd = -1;
    }

    return 0;
}

int32_t sock_connect(struct sllp_socket* sock, struct sllp_address* addr)
{
    if (!sock || !addr)
	return -1;

    struct sockaddr_in addrbuf;
    int32_t res, timeout;
    socklen_t addr_len;

    addr_len = sllp_getsocketaddrlen(sock);
    
    res = internal_connect(sock, SAS2SA(&addrbuf), addr_len, &timeout);
    if (timeout == 1)
	return -1;
    
    if (res)
	return -1;

    return 0;
}

int32_t sock_fileno(struct sllp_socket* sock)
{
    if (!sock)
	return -1;
    return sock->fd;
}

int32_t sock_getpeername(struct sllp_socket* sock, struct sllp_address* addr)
{
    if (!sock || !addr)
	return -1;

    int32_t res;
    struct sockaddr_in addrbuf;
    socklen_t addr_len;

    addr_len = sllp_getsocketaddrlen(sock);

    res = getpeername(sock->fd, SAS2SA(&addrbuf), &addr_len);
    sllp_sas2address(&addrbuf, addr);
    return 0;
}

int32_t sock_getsockname(struct sllp_socket* sock, struct sllp_address* addr)
{
    struct sockaddr_in addrbuf;
    int32_t res;
    socklen_t addr_len;
    
    addr_len = sllp_getsocketaddrlen(sock);
    res = getsockname(sock->fd, SAS2SA(&addrbuf), &addr_len);
    if (res < 0)
	return -1;
    sllp_sas2address(&addrbuf, addr);
    return 0;
}

int32_t sock_getsocketopt(struct sllp_socket* sock, u_int32_t level, u_int32_t optname, void* buf, socklen_t* buf_len)
{
    return getsockopt(sock->fd, level, optname, buf, buf_len);
}

int32_t sock_listen(struct sllp_socket* sock, u_int32_t backlog)
{
    return listen(sock->fd, backlog);
}

int32_t sock_recv(struct sllp_socket* sock, u_int8_t *buf, u_int32_t bufsize, u_int32_t flags)
{
    int32_t outlen;
    outlen = sock_recv_guts(sock, buf, bufsize, flags);
    return outlen;
}

int32_t sock_recvfrom(struct sllp_socket* sock, u_int8_t *buf, u_int32_t bufsize, u_int32_t flags, struct sllp_address *addr)
{
    int32_t outlen;
    outlen = sock_recvfrom_guts(sock, buf, bufsize, flags, addr);
    return outlen;
}

int32_t sock_send(struct sllp_socket* sock, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag)
{
    int32_t send_len, timeout;

    /* waiting until could be written  */
    timeout = internal_select(sock, 1);
    if (!timeout)
	send_len = send(sock->fd, buf, bufsize, flag);

    if (timeout == 1)
	return -1;	/* timeout */
    
    if (send_len < 1)
	return -1;	/* TODO: make an explict error */

    return 0;
}

int32_t sock_sendall(struct sllp_socket* sock, u_int8_t *buf, u_int32_t len, u_int32_t flag)
{
    int32_t timeout, n;

    do{
	n = -1;
	timeout = internal_select(sock, 1);
	if (timeout)
	    break;	/* timeout */
	n = send(sock->fd, buf, len, flag);
	
	if (n < 0)
	    break; 	/* socket occur error */
	
	buf += n;
	len -= n;
    }while(len > 0);

    if (timeout == 1)
	return -1;	/* timeout */

    if (n < 0)
	return -1;

    return 0;
}

int32_t sock_sendto(struct sllp_socket* sock, u_int8_t *buf, u_int32_t bufsize, u_int32_t flag, struct sllp_address* addr)
{
    struct sockaddr_in addrbuf;
    int32_t addr_len, timeout, outlen;

    addr_len = sllp_getsocketaddrlen(sock);
    sllp_address2sas(addr, &addrbuf);

    /* waiting for writing */
    timeout = internal_select(sock, 1);
    if (!timeout)
	outlen = sendto(sock->fd, buf, bufsize, flag, SAS2SA(&addrbuf), addr_len);

    if (timeout == 1)
	return -1;	/* timeout */

    if (outlen < 0)
	return -1;	/* socket error */
    return outlen;
}

int32_t sock_setblocking(struct sllp_socket* sock, u_int8_t block)
{
    if (block)
	sock->timeout = -1;
    else
	sock->timeout = 0;

    internal_setblocking(sock, block);
    return 0;
}

void sock_settimeout(struct sllp_socket* sock, u_int32_t value)
{
    sock->timeout = value;
}

int32_t sock_gettimeout(struct sllp_socket* sock)
{
    return sock->timeout;
}

int32_t sock_setsockopt(struct sllp_socket* sock, u_int32_t level, u_int32_t optname, void* value, int32_t len)
{
    return setsockopt(sock->fd, level, optname, value, len);
}

int32_t sock_shutdown(struct sllp_socket* sock, u_int32_t how)
{
    return shutdown(sock->fd, how);
}

struct sllp_socket* sllp_create_socket(u_int8_t family, u_int8_t type, u_int8_t protocol)
{
    SOCKET_T fd;

    fd = socket(family, type, protocol);
    if (fd < 0)
	return NULL;

    return new_sllp_socket(fd, family, type, protocol);    
}

u_int32_t sllp_free_socket(struct sllp_socket *sock)
{
    sock->close(sock);
    free(sock);
    return 0;
}

