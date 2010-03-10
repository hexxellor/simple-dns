#include "utilies.h"
#include <sys/types.h>

static void dump_dns_header(struct dns_header* header, char* buf, int bufSize)
{
    if (!header)
	return;

    u_int8_t f_qr, f_opcode, f_aa, f_tc, f_rd, f_ra, f_zero, f_rcode;

    dns.id = ntohs(dns.id);		/* ident the packet, dont chage it (c2s & s2c) */
    dns->number_questions = ntohs(dns->number_questions);
    dns->number_answers = ntohs(dns->number_answers);
    dns->number_authority = ntohs(dns->number_authority);
    dns->number_additional = ntohs(dns->number_additional);
    f_qr = dns->flag_qr;		/* 0: query packet, 1: reply packet (c2s & s2c) */
    f_opcode = dns->flag_opcode;	/* 0: defalut, 1: reverse query, 2: query the server station (c2s) */
    f_aa = dns->flag_aa;		/* 1: authoriative answers, 0: others (s2c) */
    f_tc = dns->flag_tc;		/* 1: can be truncated (c2c) */
    f_rd = dns->flag_rd;		/* 1: recursion desired, 0: not (c2s & s2c) [not sure] */
    f_ra = dns->flag_ra;		/* 1: recursable [not sure] */
    f_zero = dns->flag_zero;		/* must be 0 */
    f_rcode = dns->flag_rcode;		/* 0: success, 1: error */
}

static void dump_dns_question(struct dns_question* question, char* buf, int bufSize)
{
    
}

static void dump_dns_answer(struct dns_answer* answer, char* buf, int bufSize)
{

}

void dump_dns_query(struct dns_query* query, char* buf, int bufSize)
{
    if (!query)
	return;

    /**
     *	TODO:
     * 	step1: dump the dns query header
     *	step2: dump the dns query question
     */
}

void dump_dns_reply(struct dns_reply* reply, char* buf, int bufSize)
{
    if (!reply)
	return;

    /**
     *	TODO:
     * 	step1: dump the dns query header
     *	step2: dump the dns query question
     *	step3: dump the dns query answer
     */
}
