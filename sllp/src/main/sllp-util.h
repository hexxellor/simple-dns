#ifndef __SLLP_UTIL_H_
#define __SLLP_UTIL_H_

#include "sllp-global.h"

struct sllp_list{
	u_long* header;
	u_long* next;
	u_int32_t count;
};
u_int32_t sllp_ds_list_append(struct sllp_list* list, u_long new);
u_int32_t sllp_ds_list_remove(struct sllp_list* list, u_long sock);	

#endif
