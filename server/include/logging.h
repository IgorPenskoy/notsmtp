#ifndef __LOGGING_H__
#define __LOGGING_H__

#include <errno.h>
#include <mqueue.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "config.h"

#define LOG_QUEUE_NAME "/logging_queue"
#define LOG_MSG_STOP "LOGGING STOP"
#define LOG_TIME_SIZE 22
#define LOG_TAG_SIZE 32
#define LOG_BUF_SIZE 1024
#define LOG_FILE_ERROR (201)
#define LOG_QUEUE_ERROR (202)
#define LOG_SLEEP_MCS (100)
#define LOG_RETRY_COUNT (3)

#define LOG_INFO "[INFO]"
#define LOG_ERROR "[ERROR]"
#define LOG_NO_TAG ""
#define LOG_CMD "[%s] COMMAND: %s"
#define LOG_RESP "[%s] RESPONSE: %s"
#define LOG_DATA "DATA %s"

extern int NO_LOG;

int logging_loop(const char *logfile_name);
void write_log(char *tag, char *message, ...);
void log_sys_error(char *message);
void log_stop();

#endif // __LOGGING_H__
