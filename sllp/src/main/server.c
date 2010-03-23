#include "sllp-socketserver.h"
#include "sllp-util.h"

int8_t create_server(void)
{
    struct sllp_server* server;
    struct sllp_address addr;

    /* make the sllp style format address */
    sllp_make_address("0.0.0.0", 1234, &addr);

    /* create the sllp_server */
    server = sllp_create_tcpserver(&addr);

    /* set the function hooker */
    server->__process_request = sllp_process_request_echo;
    server->__handle_error = sllp_dummy_handle_error;
    server->__handle_timeout = sllp_dummy_handle_timeout;
    server->__verify_request = sllp_dummy_verify_request;

    server->server_forever(server, 0);

    sllp_free_tcpserver(server);
    return 0;
}
