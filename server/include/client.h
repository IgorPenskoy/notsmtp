#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <sys/queue.h>
#include <sys/select.h>

#include "config.h"
#include "fsm.h"
#include "logging.h"
#include "sds.h"
#include "smtp_cmd_utils.h"

#define RESPONSE_SIZE (1024)
#define TMP_BUF_SIZE (1024)
#define ERR_BUF_SIZE (1024)
#define TIMEOUT_SEC (60 * 5)

typedef struct smtp_args_t {
    char data[DATA_SIZE];
    char domain[DOMAIN_SIZE];
    char from[ADDRESS_SIZE];
    char rcpt[RCPT_COUNT][ADDRESS_SIZE];
    char rcpt_last[ADDRESS_SIZE];
    int rcpt_count;
} smtp_args_t;

typedef struct client_t {
    char ip_str[INET6_ADDRSTRLEN];
    int close_conn;
    int sockfd;
    sds cmd_buffer;
    sds resp_buffer;
    struct myfsm fsm;
    struct smtp_args_t args;
    time_t last_active;
    TAILQ_ENTRY(client_t) entries;
} client_t;

TAILQ_HEAD(, client_t) client_list_head;

void client_init(int sockfd, client_t **client_out);
void client_timeouts(fd_set *write_master_set);
client_t *client_get(int sockfd);
void client_args_mail(client_t *client);
void client_args_rset(client_t *client);
void client_free(client_t *client);
void client_delete(int sockfd);
void client_list_free();

#endif // __CLIENT_H__
