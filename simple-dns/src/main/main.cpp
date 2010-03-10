#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/types.h>
#include <boost/asio.hpp>
#include <string.h>

#include "utilies.h"

using boost::asio::ip::udp;

#define K(x) \
    1024 * x

#define MAX_UDP_PACKET_SIZE	K(8)

void server(boost::asio::io_service& io_service, short port)
{
    udp::socket sock(io_service, udp::endpoint(udp::v4(), port));
    u_int8_t* buffer = (u_int8_t*)malloc(MAX_UDP_PACKET_SIZE);
    u_int8_t* current;
    for (;;)
    {
	current = buffer;
	memset(current, 0, MAX_UDP_PACKET_SIZE);	
	/* step1: receive the whold packet */
	udp::endpoint sender_endpoint;
	size_t query_len = sock.receive_from(boost::asio::buffer(current, MAX_UDP_PACKET_SIZE), sender_endpoint);
	if (!query_len)
	    continue;
	printf("step1: receive %dBytes data, [ok]\n", query_len);

	struct dns_query query;
	struct dns_reply reply;

	struct dns_header header;	/* used by query and reply */
	struct dns_question question;	/* used by query and reply */
	struct dns_answer answer;	/* only used by reply */

	query.header = &header;
	query.question = &question;
	
	reply.header = &header;
	reply.question = &question;
	reply.answer = &answer;
	
	/* step2: get the dns header */	
	memcpy(query.header, current, sizeof(struct dns_header));
	current += sizeof(struct dns_header);
	printf("step2: parse dns header, [ok]\n");

	/* step3: get the dns question */
	bzero(query.question->name, sizeof(query.question->name));
	u_int8_t len; char* name = query.question->name;
	do {
	    memcpy(&len, current, sizeof(u_int8_t));
	    current += sizeof(u_int8_t);
	    if (len)
	    {
		memcpy(name, current, len);
		current += len;
		name += len;
	    }
	} while (len); 	/* receive the question name */	
	printf("step3: parser query name: %s, [ok]\n", query.question->name);	

	/* parse the question type and class */
	memcpy(&query.question->type, current, sizeof(u_int16_t));
	current += sizeof(u_int16_t);
	memcpy(&query.question->clss, current, sizeof(u_int16_t));
	current += sizeof(u_int16_t);

	/* step4: dump the dns query(optional) */
	printf("step4: (optional), [ok]\n");

	u_int8_t ip_addr[4];
	ip_addr[0] = 64;
	ip_addr[1] = 233;
	ip_addr[2] = 189;
	ip_addr[3] = 147;

	/* step5: make the reply */
  	reply.header->flag 	= htons(0x8180);
	reply.header->number_answers = htons(1);

	reply.answer->name	= htons(0xc00c);
	reply.answer->type	= htons(1);
	reply.answer->clss	= htons(1);
	reply.answer->ttls	= htonl(15 * 60); /* 15 minutes */ 
	reply.answer->data_len	= htons(4);
	reply.answer->data	= ip_addr;
	printf("step5: make the reply, [ok]\n");
	
	/* step6: preparing sending reply */
        /* we only need change the header */
	current = buffer; 
	memcpy(current, reply.header, sizeof(struct dns_header)); 

	current += query_len;
	memcpy(current, reply.answer, DNS_ANSWER_FIX_LENGHT);
	current += DNS_ANSWER_FIX_LENGHT;
	memcpy(current, reply.answer->data, 4);
	printf("step6: prepare sending reply, [ok]\n");

	sock.send_to(boost::asio::buffer(buffer, query_len + DNS_ANSWER_FIX_LENGHT + 4), sender_endpoint);
	printf("step7: send reply, [ok]\n");

	printf("==================================================================\n");
    }
    free(buffer);
}

int main(int argc, char* argv[])
{
    try
    {
	boost::asio::io_service io_service;
	using namespace std; // For atoi.
	server(io_service, 53); /* dns port */
    }
    catch (std::exception& e)
    {
	std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
