#include "sllp-socket.h"
#include <assert.h>
#include <signal.h>
#include <unistd.h>

u_int8_t snif_abort = 0;

void sighandler(int dummy)
{
    snif_abort = 1;
}

int8_t create_snif(void)
{
    struct sllp_socket* sock;
    struct sllp_address addr;
    u_int8_t buf[1024];
    int8_t len;

    signal(SIGINT, sighandler);

    sock = sllp_create_socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    assert(sock);

    sock->setsockopt(sock, IPPROTO_IP, IP_HDRINCL, (void*)&ON, sizeof(ON));
    sock->setblocking(sock, ON);

    printf("snif start\n");
    while (!snif_abort)
    {
	memset(buf, 0, sizeof(buf));
	len = sock->recvfrom(sock, buf, sizeof(buf), 0, &addr);
	if (len != -1)
	    printf("recv data from: (%s), data len: %d\n", addr.host, len);
	usleep(100);	/* sleep 100 ms */
    }

    printf("snif stop\n");

    sllp_free_socket(sock);
    
    return 0;
}
