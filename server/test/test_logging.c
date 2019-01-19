#include "test_logging.h"

int ends_with(const char *str, const char *suffix) {
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

TEST_FUNCT(log) {
    NO_LOG = 0;
    struct mq_attr attr;
    memset(&attr, 0, sizeof(struct mq_attr));
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = LOG_BUF_SIZE;
    attr.mq_curmsgs = 0;
    mqd_t mq = mq_open(LOG_QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0644,
                   &attr);
    write_log(LOG_INFO, "MESSAGE");
    char message[LOG_BUF_SIZE];
    memset(message, 0, LOG_BUF_SIZE);
    mq_receive(mq, message, LOG_BUF_SIZE, 0);
    CU_ASSERT_TRUE(ends_with(message, "MESSAGE"));
    log_sys_error("SYS ERROR");
    memset(message, 0, LOG_BUF_SIZE);
    mq_receive(mq, message, LOG_BUF_SIZE, 0);
    CU_ASSERT_TRUE(ends_with(message, "SYS ERROR"));
    log_stop();
    memset(message, 0, LOG_BUF_SIZE);
    mq_receive(mq, message, LOG_BUF_SIZE, 0);
    CU_ASSERT_TRUE(ends_with(message, LOG_MSG_STOP));
    mq_close(mq);
    mq_unlink(LOG_QUEUE_NAME);
}

int add_to_suit_logging_test(CU_pSuite suite) {
    if (suite) {
        ADD_SUITE_TEST(suite, log);
    }
    return CU_get_error();
}
