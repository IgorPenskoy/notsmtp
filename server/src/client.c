#include "client.h"

void _client_get_addr(client_t *client) {
    struct sockaddr addr;
    socklen_t addr_size = sizeof(addr);
    if (getpeername(client->sockfd, &addr, &addr_size) == 0) {
        if (addr.sa_family == AF_INET) {
            struct sockaddr_in *pV4Addr = (struct sockaddr_in *)&addr;
            struct in_addr ipAddr = pV4Addr->sin_addr;
            inet_ntop(AF_INET, &ipAddr, client->ip_str, INET_ADDRSTRLEN);
        } else {
            struct sockaddr_in6 *pV6Addr = (struct sockaddr_in6 *)&addr;
            struct in6_addr ipAddr = pV6Addr->sin6_addr;
            inet_ntop(AF_INET6, &ipAddr, client->ip_str, INET6_ADDRSTRLEN);
            if (!strcmp("::", client->ip_str)) {
                snprintf(client->ip_str, INET6_ADDRSTRLEN, "::1");
            }
        }
    }
}

void client_args_init(client_t *client) {
    client->args.rcpt_count = 0;
    memset(client->args.domain, 0, DOMAIN_SIZE);
    memset(client->args.from, 0, ADDRESS_SIZE);
    for (int i = 0; i < RCPT_COUNT; i++) {
        memset(client->args.rcpt[i], 0, ADDRESS_SIZE);
    }
    memset(client->args.rcpt_last, 0, ADDRESS_SIZE);
    memset(client->args.data, 0, DATA_SIZE);
}

void client_args_mail(client_t *client) {
    client->args.rcpt_count = 0;
    for (int i = 0; i < RCPT_COUNT; i++) {
        memset(client->args.rcpt[i], 0, ADDRESS_SIZE);
    }
    memset(client->args.rcpt_last, 0, ADDRESS_SIZE);
    memset(client->args.data, 0, DATA_SIZE);
}

void client_args_rset(client_t *client) {
    client->args.rcpt_count = 0;
    memset(client->args.from, 0, ADDRESS_SIZE);
    for (int i = 0; i < RCPT_COUNT; i++) {
        memset(client->args.rcpt[i], 0, ADDRESS_SIZE);
    }
    memset(client->args.rcpt_last, 0, ADDRESS_SIZE);
    memset(client->args.data, 0, DATA_SIZE);
}

void client_timeouts(fd_set *write_master_set) {
    client_t *tmp_client;
    time_t now = time(NULL);
    TAILQ_FOREACH(tmp_client, &client_list_head, entries) {
        if (!tmp_client->close_conn &&
            (now - tmp_client->last_active) > TIMEOUT_SEC) {
            tmp_client->resp_buffer =
                sdscatfmt(tmp_client->resp_buffer, RESP_LIST[CLOSING], HOST);
            tmp_client->close_conn = 1;
            FD_SET(tmp_client->sockfd, write_master_set);
            myfsm_advance(&tmp_client->fsm, TIME_EV, NULL, 0);
            write_log(LOG_INFO, "Closing connection %s cause of timeout",
                      tmp_client->ip_str);
        }
    }
}

void client_init(int sockfd, client_t **client_out) {
    client_t *client = (client_t *)malloc(sizeof(client_t));
    client->sockfd = sockfd;
    client->close_conn = 0;
    client->cmd_buffer = sdsempty();
    client->resp_buffer = sdsempty();
    client->resp_buffer =
        sdscatfmt(client->resp_buffer, RESP_LIST[READY], HOST);
    client->last_active = time(NULL);
    myfsm_init(&client->fsm, NULL, 0);
    _client_get_addr(client);
    client_args_init(client);
    TAILQ_INSERT_TAIL(&client_list_head, client, entries);
    write_log(LOG_INFO, "New incoming connection - %s", client->ip_str);
    (*client_out) = client;
}

client_t *client_get(int sockfd) {
    client_t *res_client = NULL;
    client_t *tmp_client;
    TAILQ_FOREACH(tmp_client, &client_list_head, entries) {
        if (tmp_client->sockfd == sockfd) {
            res_client = tmp_client;
            break;
        }
    }
    return res_client;
}

void client_free(client_t *client) {
    sdsfree(client->cmd_buffer);
    sdsfree(client->resp_buffer);
    TAILQ_REMOVE(&client_list_head, client, entries);
    free(client);
}

void client_delete(int sockfd) {
    client_t *client = client_get(sockfd);
    client_free(client);
}

void client_list_free() {
    client_t *client;
    while ((client = TAILQ_FIRST(&client_list_head))) {
        TAILQ_REMOVE(&client_list_head, client, entries);
        client_free(client);
    }
}
