#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <libconfig.h>
#include <sys/types.h>
#include <unistd.h>

#include "constants.h"

#define CONFIG_PATH "config.cfg"
#define FILE_PATH_SIZE (256)
#define HOST_NAME_SIZE (256)

extern pid_t PARENT_PID;
extern int PORT;
extern int WORKER_COUNT;
extern int UID;
extern int GID;
extern char MAILDIR[FILE_PATH_SIZE];
extern char LOGFILE[FILE_PATH_SIZE];
extern char HOST[HOST_NAME_SIZE];
extern char MAILDIR_TMP[FILE_PATH_SIZE];
extern char MAILDIR_NEW[FILE_PATH_SIZE];
extern char MAILDIR_CUR[FILE_PATH_SIZE];

int read_config(char *cfg_filename);
void print_config(FILE *f);

#endif // __CONFIG_H__
