#include "socket_utils.h"

void _port_init(char *port, int _port) {
    memset(port, 0, PORT_SIZE + 1);
    snprintf(port, PORT_SIZE + 1, "%d", _port);
}

void _hints_init(struct addrinfo *hints) {
    memset(hints, 0, sizeof(struct addrinfo));
    hints->ai_family = AF_UNSPEC;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE;
}

int _host_info_init(char *port_str, struct addrinfo *hints,
                    struct addrinfo **hostinfo) {
    if (getaddrinfo(NULL, port_str, hints, hostinfo) != 0) {
        log_sys_error("getaddrinfo() failed");
        return ERROR_RET;
    }
    return SUCCESS_RET;
}

int _socket_fcntl_init(int sockfd) {
    int sock_flags = fcntl(sockfd, F_GETFL, 0);
    if (fcntl(sockfd, F_SETFL, sock_flags | O_NONBLOCK) == -1) {
        log_sys_error("fcntl() failed");
        close(sockfd);
        return ERROR_RET;
    }
    return SUCCESS_RET;
}

int _socket_flags_init(int sockfd) {
    int switcher = 1;
    if (setsockopt(sockfd, SOL_SOCKET, (SO_REUSEADDR | SO_REUSEPORT), &switcher,
                   sizeof(switcher)) == -1) {
        log_sys_error("setsockopt() failed");
        close(sockfd);
        return ERROR_RET;
    }
    return _socket_fcntl_init(sockfd);
}

int _socket_bind(int sockfd, struct addrinfo *p) {
    if (bind(sockfd, (struct sockaddr *)p->ai_addr, p->ai_addrlen) == -1) {
        log_sys_error("bind() failed");
        close(sockfd);
        return ERROR_RET;
    }
    return SUCCESS_RET;
}

int _socket_listen(int sockfd) {
    if (listen(sockfd, BACKLOG_MAX) == -1) {
        log_sys_error("listen() failed");
        close(sockfd);
        return ERROR_RET;
    }
    return SUCCESS_RET;
}

void _socket_list_init(socket_list_t *socket_list) {
    socket_list->sockfds = NULL;
    socket_list->sockfd_max = 0;
}

int _socket_list_check(socket_list_t *socket_list) {
    if (socket_list->sockfds == NULL) {
        write_log(LOG_ERROR, "No sockets available");
        return ERROR_RET;
    }
    return SUCCESS_RET;
}

void _socket_list_add(int sockfd, socket_list_t *socket_list) {
    int_ll_t *new_sockfd = (int_ll_t *)malloc(sizeof(int_ll_t));
    new_sockfd->d = sockfd;
    new_sockfd->next = socket_list->sockfds;
    socket_list->sockfds = new_sockfd;
}

void socket_list_free(socket_list_t *socket_list) {
    int_ll_t *p = socket_list->sockfds;
    while (p != NULL) {
        int_ll_t *tmp = p->next;
        free(p);
        p = tmp;
    }
}

void fd_set_init(fd_set *master_set, socket_list_t socket_list) {
    FD_ZERO(master_set);
    int_ll_t *p = NULL;
    for (p = socket_list.sockfds; p != NULL; p = p->next) {
        FD_SET(p->d, master_set);
    }
}

void fd_set_cpy(fd_set *master_set, fd_set *working_set) {
    FD_ZERO(working_set);
    memcpy(working_set, master_set, sizeof(*master_set));
}

void fd_set_free(int max_sd, fd_set *set) {
    for (int i = 0; i <= max_sd; i++) {
        if (FD_ISSET(i, set)) {
            close(i);
        }
    }
}

int init_sockets(int port, socket_list_t *socket_list) {
    char port_str[PORT_SIZE + 1];
    _port_init(port_str, port);
    struct addrinfo hints;
    _hints_init(&hints);
    struct addrinfo *hostinfo = NULL;
    if (_host_info_init(port_str, &hints, &hostinfo) == ERROR_RET) {
        return SOCK_INIT_ERROR;
    }
    _socket_list_init(socket_list);
    struct addrinfo *p = NULL;
    for (p = hostinfo; p != NULL; p = p->ai_next) {
        int sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            continue;
        }
        if (_socket_flags_init(sockfd) == ERROR_RET) {
            return SOCK_INIT_ERROR;
        }
        if (_socket_bind(sockfd, p) == ERROR_RET) {
            continue;
        }
        if (_socket_listen(sockfd) == ERROR_RET) {
            return SOCK_INIT_ERROR;
        }
        if (sockfd > socket_list->sockfd_max) {
            socket_list->sockfd_max = sockfd;
        }
        _socket_list_add(sockfd, socket_list);
    }
    if (_socket_list_check(socket_list) == ERROR_RET) {
        return SOCK_INIT_ERROR;
    }
    freeaddrinfo(hostinfo);
    return SUCCESS_RET;
}

static int accept_count = 0;

int accept_client(int sockfd, int *max_sd, fd_set *read_master_set,
                  fd_set *write_master_set) {
    do {
        int new_sd = accept(sockfd, NULL, NULL);
        if (new_sd < 0) {
            if (errno != EWOULDBLOCK) {
                log_sys_error("accept() failed");
                return ERROR_RET;
            }
            break;
        }
        if (_socket_fcntl_init(new_sd) == ERROR_RET) {
            return ERROR_RET;
        }
        FD_SET(new_sd, read_master_set);
        FD_SET(new_sd, write_master_set);
        if (new_sd > (*max_sd)) {
            (*max_sd) = new_sd;
        }
        client_t *client;
        client_init(new_sd, &client);
        accept_count++;
        if (accept_count > ACCEPT_COUNT) {
            write_log(LOG_INFO, "Too many connections, closing connection %s",
                      client->ip_str);
            sdsclear(client->resp_buffer);
            client->resp_buffer =
                sdscatfmt(client->resp_buffer, RESP_LIST[NOT_AVAILABLE], HOST);
            client->close_conn = 1;
        }
    } while (TRUE);
    return SUCCESS_RET;
}

void close_connection(int sockfd, fd_set *read_set, fd_set *write_set,
                      int *max_sd) {
    close(sockfd);
    FD_CLR(sockfd, read_set);
    FD_CLR(sockfd, write_set);
    if (sockfd == (*max_sd)) {
        while (FD_ISSET((*max_sd), read_set) == FALSE) {
            (*max_sd)--;
        }
    }
    accept_count--;
}
