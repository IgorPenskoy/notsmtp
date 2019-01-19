#include "test_smtp_cmd_utils.h"

TEST_FUNCT(init_re) {
    init_re();
    free_re();
}

TEST_FUNCT(substring_re) {
    init_re();
    char substr[ADDRESS_SIZE];
    memset(substr, 0, ADDRESS_SIZE);
    int rc = substring_re("HELO aaa\r\n", HELO_CMD, ARG_LIST[DOMAIN_ARG], substr, ADDRESS_SIZE);
    CU_ASSERT_EQUAL(rc, ARGS_OK);
    CU_ASSERT_STRING_EQUAL(substr, "aaa");
    rc = substring_re("HELLLLLO aaa\r\n", HELO_CMD, ARG_LIST[DOMAIN_ARG], substr, ADDRESS_SIZE);
    CU_ASSERT_EQUAL(rc, ARGS_PCRE_EXEC);
    rc = substring_re("EHLO [1.1.1.1]\r\n", EHLO_CMD, ARG_LIST[DOMAIN_ARG], substr, ADDRESS_SIZE);
    CU_ASSERT_EQUAL(rc, ARGS_PCRE_SUBSTRING);
    free_re();
}

TEST_FUNCT(is_host_ok) {
    CU_ASSERT_TRUE(is_host_ok("localhost", "127.0.0.1"));
    CU_ASSERT_FALSE(is_host_ok("ip6-localhost", "127.0.0.1"));
    CU_ASSERT_FALSE(is_host_ok("aaa", "127.0.0.1"));
}

TEST_FUNCT(write_message) {
    snprintf(MAILDIR, FILE_PATH_SIZE, "/home/ip/mail/");
    snprintf(MAILDIR_TMP, FILE_PATH_SIZE, "/home/ip/mail/tmp");
    snprintf(MAILDIR_NEW, FILE_PATH_SIZE, "/home/ip/mail/new");
    snprintf(MAILDIR_CUR, FILE_PATH_SIZE, "/home/ip/mail/cur");
    int rc = write_message("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
    CU_ASSERT_EQUAL(rc, SUCCESS_RET);
}

int add_to_suit_smtp_cmd_utils_test(CU_pSuite suite) {
    if (suite) {
        ADD_SUITE_TEST(suite, init_re);
        ADD_SUITE_TEST(suite, substring_re);
        ADD_SUITE_TEST(suite, is_host_ok);
        ADD_SUITE_TEST(suite, write_message);
    }
    return CU_get_error();
}
