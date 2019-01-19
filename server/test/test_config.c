#include "test_config.h"

TEST_FUNCT(read_config) {
    char *cfg_filename = "test_config.txt";
    FILE *f = fopen(cfg_filename, "w");
    fprintf(f, "port = 1111\nworkers = 3\nuid = 100\ngid = 200\nmaildir = \"maildir\"\nlogfile = \"logfile\"\nhost = \"host\"");
    fclose(f);
    read_config(cfg_filename);
    remove(cfg_filename);
    CU_ASSERT_EQUAL(PORT, 1111);
    CU_ASSERT_EQUAL(WORKER_COUNT, 3);
    CU_ASSERT_EQUAL(UID, 100);
    CU_ASSERT_EQUAL(GID, 200);
    CU_ASSERT_STRING_EQUAL(MAILDIR, "maildir");
    CU_ASSERT_STRING_EQUAL(LOGFILE, "logfile");
    CU_ASSERT_STRING_EQUAL(HOST, "host");
}

TEST_FUNCT(print_config) {
    char *cfg_filename = "test_config.txt";
    FILE *f = fopen(cfg_filename, "w");
    fprintf(f, "port = 1111\nworkers = 3\nuid = 100\ngid = 200\nmaildir = \"maildir\"\nlogfile = \"logfile\"\nhost = \"host\"");
    fclose(f);
    read_config(cfg_filename);
    f = fopen(cfg_filename, "w");
    print_config(f);
    fclose(f);
    char buf_config[1024];
    memset(buf_config, 0, 1024);
    f = fopen(cfg_filename, "r");
    fread(buf_config, sizeof(char), 1024, f);
    char res[1024];
    memset(res, 0, 1024);
    snprintf(res, 1024, "\n*** CONFIG ***\n\n*** PORT: 1111\n*** WORKER COUNT: 3\n*** UID: 100\n*** GID: 200\n*** MAILDIR: maildir\n*** LOG FILE: logfile\n*** HOST: host\n\n*** END CONFIG ***\n\n");
    CU_ASSERT_STRING_EQUAL(buf_config, res);
    fclose(f);
    remove(cfg_filename);
}

int add_to_suit_config_test(CU_pSuite suite) {
    if (suite) {
        ADD_SUITE_TEST(suite, read_config);
        ADD_SUITE_TEST(suite, print_config);
    }
    return CU_get_error();
}
