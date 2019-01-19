#ifndef __SMTP_H__
#define __SMTP_H__

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include "client.h"
#include "config.h"
#include "fsm.h"
#include "logging.h"
#include "sds.h"
#include "smtp_cmd.h"
#include "smtp_cmd_utils.h"

int smtp_read(int sockfd, int *close_conn, fd_set *write_master_set);
int smtp_write(int sockfd, int *close_conn, fd_set *write_master_set);

static const int cmd_to_event[COMMAND_COUNT] = {
    HELO_EV, EHLO_EV, MAIL_EV, RCPT_EV, DATA_EV, RSET_EV,
    VRFY_EV, -1,      -1,      NOOP_EV, QUIT_EV};

#endif // __SMTP_H__
