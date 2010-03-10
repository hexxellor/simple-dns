#ifndef __UTILIES_H_
#define __UTILIES_H_

#pragma pack(push, 1)
struct dns_header
{
    u_int16_t id;
    u_int16_t flag;
    u_int16_t number_questions;
    u_int16_t number_answers;
    u_int16_t number_authority;
    u_int16_t number_additional;
};

struct dns_question
{
    char name[256];
    u_int16_t 	type;	/* type of query */
    u_int16_t 	clss;	/* Class of query, if address be queried is IP, it is 1;*/
};

struct dns_answer
{
    u_int16_t name;	/* normally, it's a offset of question name (s2c) */
    u_int16_t type;
    u_int16_t clss;
    int32_t ttls;
    u_int16_t data_len;
    u_int8_t  *data;
};
#define DNS_ANSWER_FIX_LENGHT	12

struct dns_reply
{
    struct dns_header		*header;
    struct dns_question		*question;
    struct dns_answer		*answer;
};

struct dns_query
{
    struct dns_header	*header;
    struct dns_question *question;
};
#pragma pack(pop)

void dump_dns_query(struct dns_query* query, char* buf, int bufSize);
void dump_dns_reply(struct dns_reply* reply, char* buf, int bufSize);

#endif
