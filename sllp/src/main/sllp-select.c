#include "sllp-select.h"

/* function declaration */
struct sllp_epoll* newepoll(int sizehint, SOCKET_T fd);
int32_t epoll_internal_close(struct sllp_epoll* epoll);
int32_t epoll_internal_ctl(int32_t epfd, struct sllp_socket* sock, int32_t cmd, int32_t events);
int32_t epoll_close(struct sllp_epoll* epoll);
int32_t epoll_regist(struct sllp_epoll* epoll, struct sllp_socket* sock, int32_t eventsmask);
int32_t epoll_modify(struct sllp_epoll *epoll, struct sllp_socket* sock, int32_t eventsmask);
int32_t epoll_unregist(struct sllp_epoll *epoll, struct sllp_socket* sock);
int32_t epoll_poll(struct sllp_epoll *epoll, int32_t timeout, int32_t maxevents, struct sllp_epoll_result* result);

int32_t seq2set(struct sllp_socket_list *list, fd_set *set);
int32_t set2list(struct sllp_socket_list* inlist, fd_set* set, struct sllp_socket_list* outlist);

void socket_list_append(struct sllp_socket_list* list, struct sllp_socket* sock);
void socket_list_remove(struct sllp_socket_list* list, struct sllp_socket* sock);
void socket_list_clean(struct sllp_socket_list* list);
u_int32_t socket_list_have(struct sllp_socket_list*, struct sllp_socket*);
/* end function declaration */

struct sllp_epoll* newepoll(int sizehint, SOCKET_T fd)
{
    struct sllp_epoll *epoll;
    epoll = malloc(sizeof(struct sllp_epoll));
    if (!epoll)
	return NULL;
    
    if (fd == -1)
	epoll->fd = epoll_create(sizehint);
    else
	epoll->fd = fd;

    if (epoll->fd < 0)
	free(epoll);	/* here we need not to use sllp_free_epoll() */
    
    epoll->close = epoll_close;
    epoll->regist = epoll_regist;
    epoll->modify = epoll_modify;
    epoll->unregist = epoll_unregist;
    epoll->poll = epoll_poll;
    return epoll;
}

int32_t epoll_internal_close(struct sllp_epoll* epoll)
{
    if (epoll->fd)
    {
	int32_t efd = epoll->fd;
	epoll->fd = -1;
	if (close(efd) < 0)
	    return -1;
    }
    return 0;
}

int32_t epoll_internal_ctl(int32_t epfd, struct sllp_socket* sock, int32_t cmd, int32_t events)
{
    struct epoll_event ev;
    int32_t result;
    SOCKET_T fd;

    fd = sock->fd;
    if (fd < 0)
	return -1;	/* invalid fd */

    switch(cmd)
    {
	case EPOLL_CTL_ADD:
	case EPOLL_CTL_MOD:
	    ev.events = events;
	    ev.data.fd = sock->fd;
	    ev.data.ptr = sock;
	    result = epoll_ctl(epfd, cmd, fd, &ev);
	    break;
	case EPOLL_CTL_DEL:
	    /* In kernel versions before 2.6.9, the EPOLL_CTL_DEL
	     * the event struct must be a non-NULL pointer, even
	     * though this argument is ignored. */
	    result = epoll_ctl(epfd, cmd, fd, &ev);
	    if (errno == EBADF)
	    {	/* fd has closed */
		result = 0;
		errno = 0;
	    }
	    break;
	default:
	    result = -1;
	    errno = EINVAL;	    
    }
    return result;
}

int32_t seq2set(struct sllp_socket_list *list, fd_set *set)
{
    int32_t i, max = -1;

    FD_ZERO(set);
    for (i = 0; i < list->count; ++i)
    {
	SOCKET_T v;
       
	v = list->socks[i]->fd;
	if (v == -1)
	    goto finally;
	if (v < 0 || v >= FD_SETSIZE)
	    goto finally;
	if (v > max)
	    max = v;
	FD_SET(v, set);
    }
    return max + 1;

  finally:
    return -1;
}

int32_t set2list(struct sllp_socket_list* inlist, fd_set* set, struct sllp_socket_list* outlist)
{
    int i;
    SOCKET_T fd;

    outlist->count = 0;
    for (i = 0; i < inlist->count; ++i)
    {
	fd = inlist->socks[i]->fd;
	if (FD_ISSET(fd, set))
	{
	    outlist->socks[outlist->count] = inlist->socks[i];
	    ++(outlist->count);
	}
    }
    return 0;
}

struct sllp_socket_list* sllp_create_socket_list()
{
    struct sllp_socket_list* slist;

    slist = malloc(sizeof(struct sllp_socket_list));
    if (!slist)
	return NULL;
    slist->count = 0;
    slist->append = socket_list_append;
    slist->remove = socket_list_remove;
    slist->clean  = socket_list_clean;
    slist->have = socket_list_have;
    return slist;
}

void sllp_free_socket_list(struct sllp_socket_list* list, int32_t on)
{
    int32_t i;
    if (on)
    {
	for (i = 0; i < list->count; ++i)
	    sllp_free_socket(list->socks[i]);
    }
    list->count = 0;
    free(list);
}

void socket_list_append(struct sllp_socket_list* list, struct sllp_socket* sock)
{
    list->socks[list->count] = sock;
    list->count++;
}

void socket_list_remove(struct sllp_socket_list* list, struct sllp_socket* sock)
{
    int32_t i;
    u_int8_t found = 0;

    for (i = 0; i < list->count; ++i)
    {
	if (list->socks[i] == sock)
	    found = 1;
	if (i == (list->count - 1))
	    break;					/* last element */
	if (found)
	    list->socks[i] = list->socks[i + 1]; 	/* left shift elemnt */
    }
    list->socks[list->count - 1] = NULL;
    --list->count;
}

void socket_list_clean(struct sllp_socket_list* list)
{
    list->count = 0;
}

u_int32_t socket_list_have(struct sllp_socket_list* list, struct sllp_socket* sock)
{
    int32_t i;

    for (i = 0; i < list->count; ++i)
    {
	if (list->socks[i] == sock)
	{
	    return 1;
	}
    }
    return 0;
}


int32_t sllp_select(struct sllp_socket_list* rlist,
		    struct sllp_socket_list* wlist, 
		    struct sllp_socket_list* elist,
		    u_int32_t timeout,
		    struct sllp_select_result* result)
{
    fd_set ifdset, ofdset, efdset;
    int32_t imax, omax, emax, max, n;
    struct timeval tv, *tvp;
    
    if (!timeout)
	tvp = NULL;	/* always wait */
    else
    {	
	tv.tv_sec  = (int32_t)timeout / 1000;		/* get seconds*/
	tv.tv_usec = (int32_t)timeout % 1000 * 1E6;	/* get ms */
	tvp = &tv;
    }

    imax = omax = emax = max = -1;

    FD_ZERO(&ifdset);
    FD_ZERO(&ofdset);
    FD_ZERO(&efdset);
    
    if (rlist && (imax = seq2set(rlist, &ifdset)) < 0)
	return -1;
    if (wlist && (omax = seq2set(wlist, &ofdset)) < 0)
	return -1;
    if (elist && (emax = seq2set(elist, &efdset)) < 0)
	return -1;

    max = imax;
    max = (omax > max) ? omax : max;
    max = (emax > max) ? emax : max;

    n = select(max, &ifdset, &ofdset, &efdset, tvp);
    if (n < 0)
	return -1;	/* go an error */
    if (n == 0)
	return 0;	/* timeout */
    else
    {
	if (result->rlist)
	{
	    result->rlist->clean(result->rlist);
	    set2list(rlist, &ifdset, result->rlist);
	}
	if (result->wlist)
	{
	    result->wlist->clean(result->wlist);
	    set2list(wlist, &ofdset, result->wlist);
	}
	if (result->elist)
	{
	    result->elist->clean(result->elist);
	    set2list(elist, &efdset, result->elist);
	}
    }
    return 1;
}

struct sllp_epoll* sllp_create_epoll(int32_t sizehint)
{
    if (sizehint == -1)
	sizehint = FD_SETSIZE - 1;
    else if (sizehint == 0)
	return NULL;

    return newepoll(sizehint, -1);
}

int32_t sllp_free_epoll(struct sllp_epoll* epoll)
{
    epoll->count = 0;
    if(epoll_internal_close(epoll) < 0)
	return -1;
    free(epoll);
    return 0;
}

int32_t epoll_close(struct sllp_epoll* epoll)
{
    return epoll_internal_close(epoll);
}

int32_t epoll_regist(struct sllp_epoll* epoll, struct sllp_socket* sock, int32_t eventsmask)
{
    return epoll_internal_ctl(epoll->fd, sock, EPOLL_CTL_ADD, eventsmask);
}

int32_t epoll_modify(struct sllp_epoll *epoll, struct sllp_socket* sock, int32_t eventsmask)
{
    return epoll_internal_ctl(epoll->fd, sock, EPOLL_CTL_MOD, eventsmask);    
}

int32_t epoll_unregist(struct sllp_epoll *epoll, struct sllp_socket* sock)
{
    return epoll_internal_ctl(epoll->fd, sock, EPOLL_CTL_DEL, 0);
}

int32_t epoll_poll(struct sllp_epoll *epoll, int32_t timeout, int32_t maxevents, struct sllp_epoll_result* result)
{
    struct epoll_event *evs;
    int32_t nfds, i;

    if (maxevents == -1)
	maxevents = FD_SETSIZE - 1;
    else if (maxevents == 0)
	return -1;	/* nothing to wait */

    evs = malloc(sizeof(struct epoll_event) * maxevents);
    
    nfds = epoll_wait(epoll->fd, evs, maxevents, timeout);
    if (nfds < 0)
    {
	free(evs);
	return -1;
    }
    
    for (i = 0; i < nfds; ++i)
    {
	result[i].sock = (struct sllp_socket*)evs[i].data.ptr;
	result[i].events = evs[i].events;
    }
    free(evs);
    return nfds;
}

