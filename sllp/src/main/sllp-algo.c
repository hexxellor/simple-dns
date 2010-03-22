#include "sllp-algo.h"

struct _list_node{
    u_long* data;
    sllp_list_node* next;
};

/*
 * Private function:
 *	static int32_t list_append(struct sllp_list* list, u_long *element);
 *	static int32_t list_remove(struct sllp_list* list, u_long *element);
 *	static int32_t list_clear(struct sllp_list* list);
 *	static int32_t list_has(struct sllp_list* list, u_long *element);
 */

static int32_t list_append(struct sllp_list* list, u_long *element)
{    
    sllp_list_node* node;

    node = malloc(sizeof(sllp_list_node));
    node->data = element;
    node->next = NULL;

    if (!list->head)
    {	/* empty element */
	list->head = node;
	list->tail = list->head;
    }
    else
    {
	assert(list->tail || !list->tail->next);
	list->tail->next = node;
	list->tail = node;
    }
    list->tail->next = NULL;
    ++(list->count);
    return 0;
}

static int32_t list_remove(struct sllp_list* list, u_long *element)
{
    sllp_list_node* node;
    sllp_list_node* prv;

    node = list->head;

    /* The element is list header, we must handle this alone */
    if (list->head->data == element)
    {
	list->head = list->head->next;
	goto finally;
    }

    while (node)
    {
	if (node->data == element)
	{/* Got it */
	    prv->next = node->next;	    
	    break;
	}
	prv = node;
	node = node->next;
    }
  finally:
    /* release the node */
    assert(node);
    free(node);
    --(list->count);
    return 0;
}

static int32_t list_clear(struct sllp_list* list)
{
    sllp_list_node* node;
    sllp_list_node* current;

    node = list->head;
    while (node)
    {
	current = node;
	node = node->next;
	free(current);
	--(list->count);
    }
    
    return 0;
}

static int32_t list_has(struct sllp_list* list, u_long *element)
{
    sllp_list_node* node;

    node = list->head;
    while (node)
    {
	if (node->data == element)
	{
	    return 1;
	}
	node = node->next;
    }
    return 0;
}

struct sllp_list* sllp_create_list()
{
    struct sllp_list* list;

    list = malloc(sizeof(struct sllp_list));
    if (!list)
	return NULL;

    list->append = list_append;
    list->remove = list_remove;
    list->clear  = list_clear;
    list->has    = list_has;

    return list;
}

int32_t sllp_free_list(struct sllp_list* list, element_free_hooker hooker)
{
    sllp_list_node* node;

    node = list->head;
    
    while (node)
    {
	if (hooker)
	    hooker(node->data);	/* free user data */
	node = node->next;
    }
    list->count = 0;
    return 0;
}
