#include "sllp-global.h"

int32_t append(struct sllp_list* list, u_long *element)
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
    return 0;
}

int32_t remove(struct sllp_list* list, u_long *element)
{

}

int32_t clear(struct sllp_list* list)
{
}

int32_t has(struct sllp_list* list, u_long *element)
{
}

