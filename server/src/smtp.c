#include "smtp.h"

int cmd_get_code(sds cmd) {
    int cmd_code = ERROR_CMD;
    for (int i = 0; i < COMMAND_COUNT && cmd_code == ERROR_CMD; i++) {
        if (!strncasecmp(cmd, CMD_LIST_NAME[i], strlen(CMD_LIST_NAME[i]))) {
            cmd_code = i;
        }
    }
    return cmd_code;
}

int not_inmplemented(int command) {
    if (command == EXPN_CMD || command == HELP_CMD) {
        return TRUE;
    }
    return FALSE;
}

void smtp_response(client_t *client, sds cmd, sds *resp) {
    if (sdslen(cmd) > CMD_SIZE) {
        (*resp) = sdscpy((*resp), RESP_LIST[TOO_LONG_LINE]);
        return;
    }
    int cmd_code = cmd_get_code(cmd);
    sdsclear((*resp));
    if (cmd_code == ERROR_CMD) {
        (*resp) = sdscpy((*resp), RESP_LIST[SYNTAX]);
        return;
    }
    if (not_inmplemented(cmd_code)) {
        (*resp) = sdscpy((*resp), RESP_LIST[NOT_IMPLEMENTED]);
        return;
    }
    int prev_state = client->fsm.current_state;
    if (myfsm_advance(&client->fsm, cmd_to_event[cmd_code], NULL, 0) !=
        CFSM_OK) {
        (*resp) = sdscpy((*resp), RESP_LIST[BAD_SEQUENCE]);
        return;
    }
    if (smtp_cmd_handle(client, cmd, cmd_code, resp) == ERROR_RET) {
        client->fsm.current_state = prev_state;
    }
}

void cmd_read(sds *cmd_buffer, sds *cmd) {
    int get_CRLF = 0;
    int buf_len = sdslen((*cmd_buffer));
    int cmd_len;
    for (cmd_len = 0; cmd_len < buf_len && !get_CRLF; cmd_len++) {
        if (cmd_len > 0 && (*cmd_buffer)[cmd_len - 1] == '\r' &&
            (*cmd_buffer)[cmd_len] == '\n') {
            get_CRLF = 1;
        }
    }
    sdsclear((*cmd));
    if (get_CRLF) {
        (*cmd) = sdscpylen((*cmd), (*cmd_buffer), cmd_len);
        sdsrange((*cmd_buffer), cmd_len, -1);
    }
}

int smtp_message(client_t *client) {
    for (int i = 0; i < client->args.rcpt_count; i++) {
        char message[MESSAGE_SIZE];
        char original_to[MESSAGE_SIZE];
        char received[MESSAGE_SIZE];
        snprintf(original_to, MESSAGE_SIZE, "X-Original-To: %s\r\n",
                 client->args.rcpt[i]);
        snprintf(received, MESSAGE_SIZE,
                 "Received: from %s ([%s]) by %s %s\r\n", client->args.domain,
                 client->ip_str, HOST, __TIMESTAMP__);
        if (client->args.from) {
            char original_from[MESSAGE_SIZE];
            snprintf(original_from, MESSAGE_SIZE, "X-Original-From: %s\r\n",
                     client->args.from);
            snprintf(message, MESSAGE_SIZE, "%s%s%s%s", original_from,
                     original_to, received, client->args.data);
        } else {
            snprintf(message, MESSAGE_SIZE, "%s%s%s", original_to, received,
                     client->args.data);
        }
        if (write_message(message) == ERROR_RET) {
            return ERROR_RET;
        }
    }
    return SUCCESS_RET;
}

void smtp_data_end(client_t *client, long int data_size) {
    write_log(LOG_INFO, "[%s] DATA RECEIVED: %d bytes", client->ip_str,
              data_size);
    const char *tmp_resp = RESP_LIST[OK];
    if (data_size + 1 > DATA_SIZE) {
        tmp_resp = RESP_LIST[TOO_MUCH_MAIL_DATA];
    } else {
        snprintf(client->args.data, data_size + 1, "%s", client->cmd_buffer);
        if (smtp_message(client) == ERROR_RET) {
            tmp_resp = RESP_LIST[SYS_STORAGE];
        }
    }
    sdsrange(client->cmd_buffer, data_size + strlen(DATA_END_STR), -1);
    myfsm_advance(&client->fsm, DATA_END_EV, NULL, 0);
    client->resp_buffer = sdscat(client->resp_buffer, tmp_resp);
    write_log(LOG_INFO, LOG_RESP, client->ip_str, tmp_resp);
}

void smtp_handle(client_t *client) {
    if (myfsm_current(&client->fsm) == DATA_STATE) {
        char *end_of_data = strstr(client->cmd_buffer, DATA_END_STR);
        if (end_of_data) {
            long int data_size = end_of_data - client->cmd_buffer;
            smtp_data_end(client, data_size);
        }
    } else {
        sds cmd = sdsempty();
        sds response = sdsempty();
        cmd_read(&client->cmd_buffer, &cmd);
        while (sdslen(cmd) > 0 && client->close_conn == 0) {
            write_log(LOG_INFO, LOG_CMD, client->ip_str, cmd);
            smtp_response(client, cmd, &response);
            write_log(LOG_INFO, LOG_RESP, client->ip_str, response);
            client->resp_buffer = sdscatsds(client->resp_buffer, response);
            if (myfsm_current(&client->fsm) == DATA_STATE) {
                sdsclear(cmd);
            } else {
                cmd_read(&client->cmd_buffer, &cmd);
            }
        }
        sdsfree(cmd);
        sdsfree(response);
    }
}

int smtp_read(int sockfd, int *close_conn, fd_set *write_master_set) {
    (*close_conn) = FALSE;
    client_t *client = client_get(sockfd);
    client->last_active = time(NULL);
    int read_size = 0;
    while (TRUE) {
        char buffer[TMP_BUF_SIZE];
        memset(buffer, 0, sizeof(buffer));
        int rsv = recv(sockfd, buffer, sizeof(buffer), 0);
        if (rsv < 0) {
            if (errno != EWOULDBLOCK) {
                log_sys_error("recv() failed");
                client_free(client);
                (*close_conn) = TRUE;
                return ERROR_RET;
            }
            break;
        } else if (rsv == 0) {
            write_log(LOG_INFO, "Connection %s closed by client",
                      client->ip_str);
            client_free(client);
            (*close_conn) = TRUE;
            return SUCCESS_RET;
        }
        client->cmd_buffer = sdscatlen(client->cmd_buffer, buffer, rsv);
        read_size += rsv;
        if (read_size > READ_SIZE) {
            write_log(LOG_INFO,
                      "Connection %s closed cause of too much incoming data",
                      client->ip_str);
            client_free(client);
            (*close_conn) = TRUE;
            return SUCCESS_RET;
        }
    }
    smtp_handle(client);
    FD_SET(sockfd, write_master_set);
    return SUCCESS_RET;
}

int smtp_write(int sockfd, int *close_conn, fd_set *write_master_set) {
    (*close_conn) = FALSE;
    client_t *client = client_get(sockfd);
    client->last_active = time(NULL);
    int buf_len = sdslen(client->resp_buffer);
    if (buf_len > 0) {
        int rsv = send(sockfd, client->resp_buffer, buf_len, 0);
        if (rsv < 0) {
            if (errno != EWOULDBLOCK) {
                log_sys_error("send() failed");
                client_free(client);
                (*close_conn) = TRUE;
                return ERROR_RET;
            }
            FD_SET(sockfd, write_master_set);
            return SUCCESS_RET;
        }
        sdsrange(client->resp_buffer, rsv, -1);
    }
    if (client->close_conn) {
        (*close_conn) = TRUE;
        write_log(LOG_INFO, "Connection %s closed, end of session",
                  client->ip_str);
        client_free(client);
    }
    FD_CLR(sockfd, write_master_set);
    return SUCCESS_RET;
}
