#ifndef __SERVER_H__
#define __SERVER_H__

#include "config.h"
#include "constants.h"
#include "logging.h"
#include "smtp_cmd_utils.h"
#include "socket.h"

#define MAX_WORKER_COUNT (8)
#define ERROR_CONFIG (101)
#define ERROR_SET_UID_GID (102)
#define ERROR_SET_WORKERS (103)

int server_run(char *config_path, int port);

#endif // __SERVER_H__
