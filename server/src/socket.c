#include "socket.h"

void process_fork(int count) {
    for (int i = 0; i < count; i++) {
        if (fork() == 0) {
            break;
        }
    }
}

int io_loop(socket_list_t socket_list) {
    fd_set read_master_set;
    fd_set_init(&read_master_set, socket_list);
    fd_set read_working_set;
    fd_set write_master_set;
    FD_ZERO(&write_master_set);
    fd_set write_working_set;
    int max_sd = socket_list.sockfd_max;
    int end_server = FALSE;
    TAILQ_INIT(&client_list_head);
    int rc = SUCCESS_RET;
    do {
        fd_set_cpy(&read_master_set, &read_working_set);
        fd_set_cpy(&write_master_set, &write_working_set);
        struct timeval select_timeout;
        select_timeout.tv_sec = 1;
        select_timeout.tv_usec = 0;
        int rsv = select(max_sd + 1, &read_working_set, &write_working_set,
                         NULL, &select_timeout);
        if (rsv < 0) {
            log_sys_error("select() failed");
            rc = SOCK_SYS_ERROR;
            break;
        }
        client_timeouts(&write_master_set);
        if (rsv == 0) {
            continue;
        }
        int desc_ready = rsv;
        for (int i = 0; i <= max_sd && desc_ready > 0; i++) {
            if (FD_ISSET(i, &read_working_set)) {
                desc_ready--;
                if (i <= socket_list.sockfd_max) {
                    if (accept_client(i, &max_sd, &read_master_set,
                                      &write_master_set) == ERROR_RET) {
                        rc = SOCK_SYS_ERROR;
                        end_server = TRUE;
                        break;
                    }
                } else {
                    int close_conn = FALSE;
                    if (smtp_read(i, &close_conn, &write_master_set) ==
                        ERROR_RET) {
                        rc = SOCK_SYS_ERROR;
                        end_server = TRUE;
                        break;
                    }
                    if (close_conn) {
                        close_connection(i, &read_master_set, &write_master_set,
                                         &max_sd);
                    }
                }
            } else if (FD_ISSET(i, &write_working_set)) {
                desc_ready--;
                int close_conn = FALSE;
                if (smtp_write(i, &close_conn, &write_master_set) ==
                    ERROR_RET) {
                    rc = SOCK_SYS_ERROR;
                    end_server = TRUE;
                    break;
                }
                if (close_conn) {
                    close_connection(i, &read_master_set, &write_master_set,
                                     &max_sd);
                }
            }
        }
    } while (end_server == FALSE);
    fd_set_free(max_sd, &read_master_set);
    fd_set_free(max_sd, &write_master_set);
    client_list_free();
    return rc;
}

int workers_start(void) {
    socket_list_t socket_list;
    if (init_sockets(PORT, &socket_list) == SOCK_INIT_ERROR) {
        return SOCK_INIT_ERROR;
    }
    process_fork(WORKER_COUNT);
    write_log(LOG_INFO, "START SMTP SERVER");
    int rc = io_loop(socket_list);
    socket_list_free(&socket_list);
    return rc;
}
