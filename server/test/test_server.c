#include "test_server.h"

TEST_FUNCT(server_run) {
    char *logfile = "unexistent_cfg.txt";
    int rc = server_run(logfile, 1111);
    CU_ASSERT_EQUAL(rc, ERROR_CONFIG);
    FILE *f = fopen(logfile, "w");
    UID = 1;
    rc = server_run(logfile, 1111);
    CU_ASSERT_EQUAL(rc, ERROR_SET_UID_GID);
    UID = 1000;
    WORKER_COUNT = -1;
    rc = server_run(logfile, 1111);
    CU_ASSERT_EQUAL(rc, ERROR_SET_WORKERS);
    fclose(f);
    remove(logfile);
}

int add_to_suit_server_test(CU_pSuite suite) {
    if (suite) {
        ADD_SUITE_TEST(suite, server_run);
    }
    return CU_get_error();
}
