#include "smtp_cmd.h"

void smtp_helo(client_t *client, sds helo_cmd, sds *resp) {
    if (is_host_ok(client->args.domain, client->ip_str)) {
        write_log(LOG_INFO, "[%s] HELO HOST (%s) IS OK", client->ip_str,
                  client->args.domain);
    } else {
        write_log(LOG_INFO, "[%s] HELO HOST (%s) IS BAD", client->ip_str,
                  client->args.domain);
    }
    client_args_rset(client);
    (*resp) = sdscpy((*resp), RESP_LIST[OK]);
}

void smtp_ehlo(client_t *client, sds ehlo_cmd, sds *resp) {
    if (is_host_ok(client->args.domain, client->ip_str)) {
        write_log(LOG_INFO, "[%s] EHLO HOST (%s) IS OK", client->ip_str,
                  client->args.domain);
    } else {
        write_log(LOG_INFO, "[%s] EHLO HOST (%s) IS BAD", client->ip_str,
                  client->args.domain);
    }
    client_args_rset(client);
    (*resp) = sdscpy((*resp), RESP_LIST[OK]);
}

void smtp_mail(client_t *client, sds helo_cmd, sds *resp) {
    client_args_mail(client);
    (*resp) = sdscpy((*resp), RESP_LIST[OK]);
}

int smtp_rcpt(client_t *client, sds helo_cmd, sds *resp) {
    const char *tmp_resp = NULL;
    int rc = SUCCESS_RET;
    if (client->args.rcpt_count >= RCPT_COUNT) {
        tmp_resp = RESP_LIST[TOO_MANY_RECIPIENTS];
        rc = ERROR_RET;
    } else {
        snprintf(client->args.rcpt[client->args.rcpt_count], ADDRESS_SIZE, "%s",
                 client->args.rcpt_last);
        client->args.rcpt_count++;
        tmp_resp = RESP_LIST[OK];
    }
    (*resp) = sdscpy((*resp), tmp_resp);
    return rc;
}

void smtp_data(client_t *client, sds helo_cmd, sds *resp) {
    (*resp) = sdscpy((*resp), RESP_LIST[START_INPUT]);
}

void smtp_rset(client_t *client, sds helo_cmd, sds *resp) {
    client_args_rset(client);
    (*resp) = sdscpy((*resp), RESP_LIST[OK]);
}

void smtp_vrfy(client_t *client, sds helo_cmd, sds *resp) {
    (*resp) = sdscpy((*resp), RESP_LIST[MAILBOX_UNAVAILABLE]);
}

void smtp_noop(client_t *client, sds helo_cmd, sds *resp) {
    (*resp) = sdscpy((*resp), RESP_LIST[OK]);
}

void smtp_quit(client_t *client, sds quit_cmd, sds *resp) {
    client->close_conn = 1;
    (*resp) = sdscatfmt((*resp), RESP_LIST[CLOSING], HOST);
}

int smtp_args_helo_handle(client_t *client, sds helo_cmd) {
    char domain[CMD_SIZE];
    int rc = substring_re(helo_cmd, HELO_CMD, ARG_LIST[DOMAIN_ARG], domain,
                          CMD_SIZE);
    if (rc) {
        return rc;
    }
    if (strlen(domain) > DOMAIN_SIZE) {
        return ARGS_TOO_MUCH;
    }
    snprintf(client->args.domain, DOMAIN_SIZE, "%s", domain);
    return rc;
}

int smtp_args_ehlo_handle(client_t *client, sds ehlo_cmd) {
    char domain[CMD_SIZE];
    int rc = substring_re(ehlo_cmd, EHLO_CMD, ARG_LIST[DOMAIN_ARG], domain,
                          CMD_SIZE);
    if (rc) {
        return rc;
    }
    if (strlen(domain) > DOMAIN_SIZE) {
        return ARGS_TOO_MUCH;
    }
    snprintf(client->args.domain, DOMAIN_SIZE, "%s", domain);
    return rc;
}

int smtp_args_mail_handle(client_t *client, sds mail_cmd) {
    char from[CMD_SIZE];
    int rc =
        substring_re(mail_cmd, MAIL_CMD, ARG_LIST[MAIL_ARG], from, CMD_SIZE);
    if (rc) {
        return rc;
    }
    if (strlen(from) > ADDRESS_SIZE) {
        return ARGS_TOO_MUCH;
    }
    snprintf(client->args.from, ADDRESS_SIZE, "%s", from);
    return rc;
}

int smtp_args_rcpt_handle(client_t *client, sds rcpt_cmd) {
    char to[CMD_SIZE];
    int rc = substring_re(rcpt_cmd, RCPT_CMD, ARG_LIST[MAIL_ARG], to, CMD_SIZE);
    if (rc) {
        return rc;
    }
    if (strlen(to) > ADDRESS_SIZE) {
        return ARGS_TOO_MUCH;
    }
    snprintf(client->args.rcpt_last, ADDRESS_SIZE, "%s", to);
    return rc;
}

int smtp_args_handle(client_t *client, sds cmd, int cmd_code) {
    switch (cmd_code) {
    case HELO_CMD:
        return smtp_args_helo_handle(client, cmd);
    case EHLO_CMD:
        return smtp_args_ehlo_handle(client, cmd);
    case MAIL_CMD:
        return smtp_args_mail_handle(client, cmd);
    case RCPT_CMD:
        return smtp_args_rcpt_handle(client, cmd);
    case DATA_CMD:
        return substring_re(cmd, DATA_CMD, NULL, NULL, 0);
        ;
    case RSET_CMD:
        return substring_re(cmd, RSET_CMD, NULL, NULL, 0);
        ;
    case VRFY_CMD:
        return substring_re(cmd, VRFY_CMD, NULL, NULL, 0);
    case NOOP_CMD:
        return substring_re(cmd, NOOP_CMD, NULL, NULL, 0);
    case QUIT_CMD:
        return substring_re(cmd, QUIT_CMD, NULL, NULL, 0);
        ;
    }
    return ARGS_ERROR;
}

int smtp_cmd_handle(client_t *client, sds cmd, int cmd_code, sds *resp) {
    int rc = smtp_args_handle(client, cmd, cmd_code);
    if (rc == ARGS_OK) {
        rc = SUCCESS_RET;
        switch (cmd_code) {
        case HELO_CMD:
            smtp_helo(client, cmd, resp);
            break;
        case EHLO_CMD:
            smtp_ehlo(client, cmd, resp);
            break;
        case MAIL_CMD:
            smtp_mail(client, cmd, resp);
            break;
        case RCPT_CMD:
            rc = smtp_rcpt(client, cmd, resp);
            break;
        case DATA_CMD:
            smtp_data(client, cmd, resp);
            break;
        case RSET_CMD:
            smtp_rset(client, cmd, resp);
            break;
        case VRFY_CMD:
            smtp_vrfy(client, cmd, resp);
            break;
        case NOOP_CMD:
            smtp_noop(client, cmd, resp);
            break;
        case QUIT_CMD:
            smtp_quit(client, cmd, resp);
            break;
        }
    } else {
        const char *tmp_resp = NULL;
        switch (rc) {
        case ARGS_PCRE_EXEC:
            tmp_resp = RESP_LIST[SYNTAX_PARAMETERS];
            break;
        case ARGS_PCRE_SUBSTRING:
            tmp_resp = RESP_LIST[PARAMETER_NOT_IMPLEMENTED];
            break;
        case ARGS_TOO_MUCH:
            tmp_resp = RESP_LIST[STORAGE_EXCEEDED];
            break;
        default:
            tmp_resp = RESP_LIST[ABORTED];
            break;
        }
        (*resp) = sdscpy((*resp), tmp_resp);
        rc = ERROR_RET;
    }
    return rc;
}
