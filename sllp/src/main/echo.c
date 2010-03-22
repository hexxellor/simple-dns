#include "sllp-socket.h"
#include "sllp-select.h"
#include <assert.h>
#include <signal.h>
#include <unistd.h>

int8_t create_echo(void)
{
    struct sllp_address addr;
    struct sllp_socket *server, *client;
    u_int32_t running, i, datalen;
    struct sllp_socket_list *rlist, *outlist;
    struct sllp_select_result result;
    u_int8_t buf[1024];

    sllp_make_address("0.0.0.0", 1234, &addr);
    server = sllp_create_socket(AF_INET, SOCK_STREAM, 0);

    server->setsockopt(server, SOL_SOCKET, SO_REUSEADDR, (void*)&ON, sizeof(ON));
    server->bind(server, &addr);
    server->listen(server, 5);

    rlist = sllp_create_socket_list();
    outlist = sllp_create_socket_list();

    rlist->append(rlist, server);
    
    result.rlist = outlist;
    result.wlist = NULL;
    result.elist = NULL;

    running = 1;
    while (running)
    {
	if(sllp_select(rlist, NULL, NULL, 1000, &result) == 1)
	{/* have event occur */
	    if (result.rlist->have(result.rlist, server))
	    {
		client = server->accept(server);
		printf("accept client from: (%s, %d)\n", client->address.host, client->address.port);
		rlist->append(rlist, client);
	    }
	    else
	    {		
		for (i = 0; i < result.rlist->count; ++i)
		{
		    client = result.rlist->socks[i];
		    memset(buf, 0, sizeof(buf));
		    datalen = client->recv(client, buf, sizeof(buf), 0);
		    if (!datalen)
		    {
			sllp_free_socket((struct sllp_socket*)client);		/* close the client */
			rlist->remove(rlist, client);				/* remove from inputlist */
		    }
		    printf("receive data from: (%s, %d), len: %d, data: %s\n", 
			   client->address.host, client->address.port, datalen, buf);
		    client->sendall(client, buf, datalen, 0);
		}
	    }
	}
    }
    sllp_free_socket_list(rlist, ON);
    sllp_free_socket_list(outlist, OFF);	/* because the rlist and outlist is share the socket objct,
						   so, when we release the rlist, the socket objct has been
						   released.
						 */

    return 0;
}
