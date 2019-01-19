#include "test_smtp_cmd.h"

TEST_FUNCT(smtp_cmd_handle) {
    NO_LOG = 1;
    init_re();
    TAILQ_INIT(&client_list_head);
    client_t *client;
    client_init(0, &client);
    sds cmd = sdsempty();
    sds resp = sdsempty();
    snprintf(client->ip_str, INET6_ADDRSTRLEN, "127.0.0.1");
    cmd = sdscpy(cmd, "HELO localhost\r\n");
    int rc = smtp_cmd_handle(client, cmd, HELO_CMD, &resp);
    CU_ASSERT_EQUAL(rc, SUCCESS_RET);
    CU_ASSERT_STRING_EQUAL(resp, "250 OK\r\n");
    cmd = sdscpy(cmd, "HELO a@b.com\r\n");
    rc = smtp_cmd_handle(client, cmd, HELO_CMD, &resp);
    CU_ASSERT_EQUAL(rc, ERROR_RET);
    sds tmp_resp = sdsnew(RESP_LIST[SYNTAX_PARAMETERS]);
    CU_ASSERT_STRING_EQUAL(resp, tmp_resp);
    sdsfree(cmd);
    sdsfree(resp);
    sdsfree(tmp_resp);
    client_delete(0);
    free_re();
}

int add_to_suit_smtp_cmd_test(CU_pSuite suite) {
    if (suite) {
        ADD_SUITE_TEST(suite, smtp_cmd_handle);
    }
    return CU_get_error();
}
