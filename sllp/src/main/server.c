#include "sllp-socketserver.h"

int8_t create_server(void)
{
    struct sllp_server* server;
    struct sllp_address addr;

    sllp_make_address("0.0.0.0", 1234, &addr);

    server = sllp_create_tcpserver(&addr);

    server->server_forever(server, 0);

    sllp_free_tcpserver(server);
    return 0;
}
