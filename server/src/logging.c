#include "logging.h"

int NO_LOG = 0;

mqd_t create_read_mq(void) {
    struct mq_attr attr;
    memset(&attr, 0, sizeof(struct mq_attr));
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = LOG_BUF_SIZE;
    attr.mq_curmsgs = 0;
    return mq_open(LOG_QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0644,
                   &attr);
}

mqd_t create_write_mq(void) {
    struct mq_attr attr;
    memset(&attr, 0, sizeof(struct mq_attr));
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = LOG_BUF_SIZE;
    attr.mq_curmsgs = 0;
    return mq_open(LOG_QUEUE_NAME, O_CREAT | O_WRONLY | O_NONBLOCK, 0644,
                   &attr);
}

int logging_loop(const char *logfile_name) {
    mqd_t mq = create_read_mq();
    if (mq == -1) {
        return LOG_QUEUE_ERROR;
    } else {
        int logging_stop = 0;
        while (!logging_stop) {
            char buffer[LOG_BUF_SIZE + 1];
            memset(buffer, 0, sizeof(buffer));
            ssize_t bytes_read = mq_receive(mq, buffer, LOG_BUF_SIZE, NULL);
            if (!strcmp(buffer, LOG_MSG_STOP)) {
                logging_stop = 1;
            } else if (bytes_read > 0) {
                FILE *f = fopen(logfile_name, "a");
                if (!f) {
                    return LOG_FILE_ERROR;
                }
                if (strchr(buffer, '\n')) {
                    fprintf(f, "%s", buffer);
                    fprintf(stdout, "%s", buffer);
                } else {
                    fprintf(f, "%s\n", buffer);
                    fprintf(stdout, "%s\n", buffer);
                }
                fclose(f);
            }
            usleep(LOG_SLEEP_MCS);
        }
        mq_close(mq);
        mq_unlink(LOG_QUEUE_NAME);
    }
    return SUCCESS_RET;
}

void timestamp(char *buffer, size_t len) {
    time_t ltime = time(NULL);
    struct tm t;
    memset(&t, 0, sizeof(struct tm));
    localtime_r(&ltime, &t);
    memset(buffer, 0, len);
    strftime(buffer, len, "[%d.%m.%Y %H:%M:%S]", &t);
}

void write_log(char *tag, char *message, ...) {
    if (NO_LOG) {
        return;
    }
    static mqd_t mq = -1;
    static int log_stop = 0;
    if (mq == -1) {
        mq = create_write_mq();
    }
    if (!log_stop && mq != -1) {
        char _timestamp[LOG_TIME_SIZE];
        timestamp(_timestamp, sizeof(_timestamp));
        char tmp_message[LOG_BUF_SIZE];
        va_list args;
        va_start(args, message);
        vsnprintf(tmp_message, sizeof(tmp_message), message, args);
        va_end(args);
        char buffer[LOG_BUF_SIZE];
        memset(buffer, 0, LOG_BUF_SIZE);
        if (strcmp(LOG_MSG_STOP, message)) {
            snprintf(buffer, sizeof(buffer), "[%d] %s %s %s", getpid(),
                     _timestamp, tag, tmp_message);
        } else {
            snprintf(buffer, sizeof(buffer), "%s", message);
        }
        int send = -1;
        for (int i = 0; i < LOG_RETRY_COUNT && send == -1; i++) {
            send = mq_send(mq, buffer, LOG_BUF_SIZE, 0);
        }
        if (!strcmp(LOG_MSG_STOP, message)) {
            if (PARENT_PID == getpid()) {
                int wait_status = 0;
                wait(&wait_status);
            }
            log_stop = 1;
            mq_close(mq);
            mq_unlink(LOG_QUEUE_NAME);
        }
    }
}

void log_sys_error(char *message) {
    char tmp_message[LOG_BUF_SIZE];
    memset(tmp_message, 0, sizeof(tmp_message));
    snprintf(tmp_message, sizeof(tmp_message), "%s %s", message,
             strerror(errno));
    write_log(LOG_ERROR, message);
}

void log_stop() { write_log(LOG_NO_TAG, LOG_MSG_STOP); }
