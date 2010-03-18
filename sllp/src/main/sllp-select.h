#ifndef __SLLP_SELECT_H_
#define __SLLP_SELECT_H_

#include "sllp-global.h"
#include "sllp-socket.h"

struct sllp_socket_list;
struct sllp_select_result;
struct sllp_epoll;
struct sllp_epoll_result;

/**
 * Just like system interface 'select'. This function return a result
 * object it contains the lists which we are waiting. 
 *
 * The first three input argument are descriptors that we want to waring. 
 *
 * If you set timeout is zero, then it will be block always until
 * return by an event occur which we are listening.
 *
 * If you set a timeout value, that the function will return if the
 * time is arrived. 
 */
int32_t sllp_select(struct sllp_socket_list* reads, 
		    struct sllp_socket_list* writes, 
		    struct sllp_socket_list* errors,
		    u_int32_t timeout,
		    struct sllp_select_result* result);

/**
 * The function will create an epoll object. The sizehint is the max
 * events we want to listen.
 */
struct sllp_epoll* sllp_create_epoll(int32_t sizehint);

/**
 * Free the epoll object that create by two functions:
 * 	* sllp_create_epoll()
 */
int32_t sllp_free_epoll(struct sllp_epoll* epoll);

/**
 * TODO:
 *     	The inser and delete opertaion is more than random access, so
 *	we need change the list from array to link list.
 */
struct sllp_socket_list{
    void (*append)(struct sllp_socket_list*, struct sllp_socket*);
    void (*remove)(struct sllp_socket_list*, struct sllp_socket*);
    void (*clean)(struct sllp_socket_list*);
    u_int32_t (*have)(struct sllp_socket_list*, struct sllp_socket*);    
    struct sllp_socket *socks[FD_SETSIZE + 1];
    u_int32_t count;
};
struct sllp_socket_list* sllp_create_socket_list();
void sllp_free_socket_list(struct sllp_socket_list* list, int32_t on);

struct sllp_select_result{
	struct sllp_socket_list* rlist;
	struct sllp_socket_list* wlist;
	struct sllp_socket_list* elist;	
};

struct sllp_epoll{
    int32_t (*close)(struct sllp_epoll*);
    int32_t (*regist)(struct sllp_epoll*, struct sllp_socket* sock, int32_t eventsmask);
    int32_t (*modify)(struct sllp_epoll*, struct sllp_socket* sock, int32_t eventsmask);
    int32_t (*unregist)(struct sllp_epoll*, struct sllp_socket* sock);
    int32_t (*poll)(struct sllp_epoll*, int32_t timeout, int32_t maxevents, struct sllp_epoll_result* result);

    int32_t	fd;
    int32_t	fdlist[FD_SETSIZE + 1];
    u_int32_t	count;
};

struct sllp_epoll_result{
    struct sllp_socket* sock;
    int32_t events;    
};

#endif
