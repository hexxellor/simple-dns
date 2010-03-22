#ifndef __SLLP_UTIL_H_
#define __SLLP_UTIL_H_

#include "sllp-global.h"
#include "sllp-socketserver.h"

int32_t sllp_process_request_echo(struct sllp_server* server, struct sllp_socket* request);
int32_t sllp_handle_error(struct sllp_server* server, struct sllp_socket* request);
int32_t sllp_handle_timeout(struct sllp_server* server);
int32_t sllp_verify_request(struct sllp_server* server, struct sllp_socket* request);
int32_t sllp_server_close(struct sllp_server* server);

#endif
