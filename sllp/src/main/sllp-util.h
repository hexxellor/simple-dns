#ifndef __SLLP_UTIL_H_
#define __SLLP_UTIL_H_

#include "sllp-global.h"
#include "sllp-socketserver.h"

int32_t sllp_process_request_echo(struct sllp_server* server, struct sllp_request* request);

int32_t sllp_dummy_handle_error(struct sllp_server* server, struct sllp_request* request);
int32_t sllp_dummy_handle_timeout(struct sllp_server* server);
int32_t sllp_dummy_verify_request(struct sllp_server* server, struct sllp_request* request);

#endif
