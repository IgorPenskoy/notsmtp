#include "config.h"

pid_t PARENT_PID;
int PORT = 2525;
int WORKER_COUNT = 4;
int UID = 1000;
int GID = 1000;
char MAILDIR[FILE_PATH_SIZE] = "/home/ip/mail/";
char MAILDIR_TMP[FILE_PATH_SIZE] = "/home/ip/mail/tmp";
char MAILDIR_NEW[FILE_PATH_SIZE] = "/home/ip/mail/new";
char MAILDIR_CUR[FILE_PATH_SIZE] = "/home/ip/mail/cur";
char LOGFILE[FILE_PATH_SIZE] = "log.txt";
char HOST[HOST_NAME_SIZE] = "localhost";

void lookup_string(config_t cfg, char *lookup, char *dest, size_t size) {
    const char *tmp_string = NULL;
    config_lookup_string(&cfg, lookup, &tmp_string);
    snprintf(dest, size, "%s", tmp_string);
}

int read_config(char *cfg_filename) {
    config_t cfg;
    config_init(&cfg);
    if (!config_read_file(&cfg, cfg_filename)) {
        config_destroy(&cfg);
        return ERROR_RET;
    }
    config_lookup_int(&cfg, "port", &PORT);
    config_lookup_int(&cfg, "workers", &WORKER_COUNT);
    config_lookup_int(&cfg, "uid", &UID);
    config_lookup_int(&cfg, "gid", &GID);
    lookup_string(cfg, "maildir", MAILDIR, sizeof(MAILDIR));
    lookup_string(cfg, "logfile", LOGFILE, sizeof(LOGFILE));
    lookup_string(cfg, "host", HOST, sizeof(HOST));
    snprintf(MAILDIR_TMP, FILE_PATH_SIZE, "%s/tmp/", MAILDIR);
    snprintf(MAILDIR_NEW, FILE_PATH_SIZE, "%s/new/", MAILDIR);
    snprintf(MAILDIR_CUR, FILE_PATH_SIZE, "%s/cur/", MAILDIR);
    config_destroy(&cfg);
    PARENT_PID = getpid();
    return SUCCESS_RET;
}

void print_config(FILE *f) {
    fprintf(f, "\n*** CONFIG ***\n\n");
    fprintf(f, "*** PORT: %d\n", PORT);
    fprintf(f, "*** WORKER COUNT: %d\n", WORKER_COUNT);
    fprintf(f, "*** UID: %d\n", UID);
    fprintf(f, "*** GID: %d\n", GID);
    fprintf(f, "*** MAILDIR: %s\n", MAILDIR);
    fprintf(f, "*** LOG FILE: %s\n", LOGFILE);
    fprintf(f, "*** HOST: %s\n", HOST);
    fprintf(f, "\n*** END CONFIG ***\n\n");
}
