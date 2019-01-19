#ifndef __SOCKET_UTILS_H__
#define __SOCKET_UTILS_H__

#include "client.h"
#include "constants.h"
#include "logging.h"

#define BACKLOG_MAX (16)
#define PORT_SIZE (5)
#define ACCEPT_COUNT (10)
#define SOCK_INIT_ERROR (301)
#define SOCK_SYS_ERROR (302)

typedef struct int_ll_t {
    int d;
    struct int_ll_t *next;
} int_ll_t;

typedef struct socket_list_t {
    struct int_ll_t *sockfds;
    int sockfd_max;
} socket_list_t;

int init_sockets(int port, socket_list_t *socket_list);
void socket_list_free(socket_list_t *socket_list);
void fd_set_init(fd_set *master_set, socket_list_t socket_list);
void fd_set_cpy(fd_set *master_set, fd_set *working_set);
void fd_set_free(int max_sd, fd_set *set);
int accept_client(int sockfd, int *max_sd, fd_set *read_master_set,
                  fd_set *write_master_set);
void close_connection(int sockfd, fd_set *read_set, fd_set *write_set,
                      int *max_sd);

#endif // __SOCKET_UTILS_H__
