#ifndef __SLLP_ALGO_H_
#define __SLLP_ALGO_H_

#include "sllp-global.h"

typedef void(*element_free_hooker)(u_long*);

typedef struct tag_list_node{
    u_long* data;
    u_long* next;
}sllp_list_node;
/*
 * Note: we use u_long to indicate pointer type. At the plantform of 32-bit,
 *	 the size of u_long is 32bit, and on 64-bit is 64bit.
 */
struct sllp_list{
    sllp_list_node* head;
    sllp_list_node* tail;
    u_int32_t count;
    int32_t (*append)(struct sllp_list*, u_long *element);
    int32_t (*remove)(struct sllp_list*, u_long *element);
    int32_t (*clear)(struct sllp_list*);
    int32_t (*has)(struct sllp_list*, u_long *element);
};
struct sllp_list* sllp_create_list();
int32_t sllp_free_list(struct sllp_list* list, element_free_hooker hooker);

#endif
