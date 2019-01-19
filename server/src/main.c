#include "config.h"
#include "server.h"
#include "shell_opt.h"

int main(int argc, char *argv[]) {
    int optct = optionProcess(&notsmtpOptions, argc, argv);
    argc -= optct;
    argv += optct;
    char config_path[256];
    snprintf(config_path, sizeof(config_path), "%s", CONFIG_PATH);
    if (HAVE_OPT(CONFIG)) {
        snprintf(config_path, sizeof(config_path), "%s", OPT_ARG(CONFIG));
    }
    int port = 0;
    if (HAVE_OPT(PORT)) {
        port = OPT_VALUE_PORT;
    }
    int rc = server_run(config_path, port);
    if (rc != SUCCESS_RET) {
        switch (rc) {
        case ERROR_CONFIG:
            printf("ERROR in config reading, check config path\n");
            break;
        case LOG_FILE_ERROR:
            printf("ERROR in logging, check log file\n");
            break;
        case LOG_QUEUE_ERROR:
            printf("ERROR in logging, logging queue not working\n");
            break;
        case SOCK_INIT_ERROR:
            perror("ERROR in socket initialization: \n");
            break;
        case SOCK_SYS_ERROR:
            perror("ERROR in socket system function: \n");
            break;
        case ERROR_SET_UID_GID:
            printf("ERROR in set UID == %d and GID == %d\n", UID, GID);
            break;
        default:
            printf("ERROR undefined\n");
            break;
        }
    }
    return rc;
}
