#include "server.h"

int server_run(char *config_path, int port) {
    if (read_config(config_path) == ERROR_RET) {
        return ERROR_CONFIG;
    }
    if (WORKER_COUNT < 0 || WORKER_COUNT > MAX_WORKER_COUNT) {
        return ERROR_SET_WORKERS;
    }
    if (setuid(UID) == -1 || setgid(GID) == -1) {
        return ERROR_SET_UID_GID;
    }
    if (port) {
        PORT = port;
    }
    print_config(stdout);
    int rc = SUCCESS_RET;
    if (fork() == 0) {
        rc = logging_loop(LOGFILE);
    } else {
        init_re();
        rc = workers_start();
        if (PARENT_PID == getpid()) {
            log_stop();
        }
        free_re();
    }
    return rc;
}
