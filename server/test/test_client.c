#include "test_client.h"

TEST_FUNCT(client_init) {
    NO_LOG = 1;
    TAILQ_INIT(&client_list_head);
    client_t *client;
    client_init(0, &client);
	CU_ASSERT_EQUAL(client->close_conn, 0);
	CU_ASSERT_EQUAL(client->sockfd, 0);
	CU_ASSERT_EQUAL(sdslen(client->cmd_buffer), 0);
    sds tmp = sdsempty();
    tmp = sdscatfmt(tmp, RESP_LIST[READY], HOST);
	CU_ASSERT_EQUAL(strcmp(client->resp_buffer, tmp), 0);
	CU_ASSERT_EQUAL(myfsm_current(&client->fsm), INIT_STATE);
    CU_ASSERT_EQUAL(client->args.rcpt_count, 0);
    CU_ASSERT_EQUAL(strlen(client->args.domain), 0);
    CU_ASSERT_EQUAL(strlen(client->args.from), 0);
    CU_ASSERT_EQUAL(strlen(client->args.rcpt[0]), 0);
    CU_ASSERT_EQUAL(strlen(client->args.data), 0);
    CU_ASSERT_EQUAL(strlen(client->args.rcpt_last), 0);
    TAILQ_REMOVE(&client_list_head, client, entries);
    CU_ASSERT_EQUAL(TAILQ_EMPTY(&client_list_head), 1);
    client_free(client);
    sdsfree(tmp);
}

TEST_FUNCT(client_timeouts) {
    NO_LOG = 1;
    TAILQ_INIT(&client_list_head);
    client_t *client;
    client_init(0, &client);
    fd_set tmp_set;
    FD_ZERO(&tmp_set);
    client_timeouts(&tmp_set);
	CU_ASSERT_EQUAL(client->close_conn, 0);
    CU_ASSERT_FALSE(FD_ISSET(client->sockfd, &tmp_set))
    client->last_active = time(NULL) - TIMEOUT_SEC - 1;
    client_timeouts(&tmp_set);
	CU_ASSERT_EQUAL(client->close_conn, 1);
    CU_ASSERT_TRUE(FD_ISSET(client->sockfd, &tmp_set))
    TAILQ_REMOVE(&client_list_head, client, entries);
    client_free(client);
}

TEST_FUNCT(client_get) {
    NO_LOG = 1;
    TAILQ_INIT(&client_list_head);
    client_t *client;
    client_init(0, &client);
    client_t *tmp_client = client_get(0);
    CU_ASSERT_PTR_NOT_NULL(tmp_client);
    tmp_client = client_get(1);
    CU_ASSERT_PTR_NULL(tmp_client);
    TAILQ_REMOVE(&client_list_head, client, entries);
    client_free(client);
}

TEST_FUNCT(client_args_mail) {
    NO_LOG = 1;
    TAILQ_INIT(&client_list_head);
    client_t *client;
    client_init(0, &client);
    client->args.rcpt_count = 1;
    snprintf(client->args.from, ADDRESS_SIZE, "%s", "a@b.com");
    snprintf(client->args.rcpt[0], ADDRESS_SIZE, "%s", "a@b.com");
    snprintf(client->args.data, DATA_SIZE, "%s", "DATA");
    snprintf(client->args.rcpt_last, ADDRESS_SIZE, "%s", "a@b.com");
    client_args_mail(client);
    CU_ASSERT_EQUAL(client->args.rcpt_count, 0);
    CU_ASSERT_EQUAL(strlen(client->args.from), 7);
    CU_ASSERT_EQUAL(strlen(client->args.rcpt[0]), 0);
    CU_ASSERT_EQUAL(strlen(client->args.data), 0);
    CU_ASSERT_EQUAL(strlen(client->args.rcpt_last), 0);
    TAILQ_REMOVE(&client_list_head, client, entries);
    client_free(client);
}

TEST_FUNCT(client_args_rset) {
    NO_LOG = 1;
    TAILQ_INIT(&client_list_head);
    client_t *client;
    client_init(0, &client);
    client->args.rcpt_count = 1;
    snprintf(client->args.from, ADDRESS_SIZE, "%s", "a@b.com");
    snprintf(client->args.rcpt[0], ADDRESS_SIZE, "%s", "a@b.com");
    snprintf(client->args.data, DATA_SIZE, "%s", "DATA");
    snprintf(client->args.rcpt_last, ADDRESS_SIZE, "%s", "a@b.com");
    client_args_rset(client);
    CU_ASSERT_EQUAL(client->args.rcpt_count, 0);
    CU_ASSERT_EQUAL(strlen(client->args.from), 0);
    CU_ASSERT_EQUAL(strlen(client->args.rcpt[0]), 0);
    CU_ASSERT_EQUAL(strlen(client->args.data), 0);
    CU_ASSERT_EQUAL(strlen(client->args.rcpt_last), 0);
    TAILQ_REMOVE(&client_list_head, client, entries);
    client_free(client);
}

TEST_FUNCT(client_free) {
    NO_LOG = 1;
    TAILQ_INIT(&client_list_head);
    client_t *client;
    client_init(0, &client);
    TAILQ_REMOVE(&client_list_head, client, entries);
    CU_ASSERT_PTR_NOT_NULL(client);
    client_free(client);
}

TEST_FUNCT(client_delete) {
    NO_LOG = 1;
    TAILQ_INIT(&client_list_head);
    client_t *client;
    client_init(0, &client);
    client_delete(0);
    CU_ASSERT_EQUAL(TAILQ_EMPTY(&client_list_head), 1);
}

TEST_FUNCT(client_list_free) {
    NO_LOG = 1;
    TAILQ_INIT(&client_list_head);
    client_t *client;
    client_init(0, &client);
    client_init(1, &client);
    client_init(2, &client);
    client_list_free();
    CU_ASSERT_EQUAL(TAILQ_EMPTY(&client_list_head), 1);
}

int add_to_suit_client_test(CU_pSuite suite) {
    if (suite) {
        ADD_SUITE_TEST(suite, client_init);
        ADD_SUITE_TEST(suite, client_timeouts);
        ADD_SUITE_TEST(suite, client_get);
        ADD_SUITE_TEST(suite, client_args_mail);
        ADD_SUITE_TEST(suite, client_args_rset);
        ADD_SUITE_TEST(suite, client_free);
        ADD_SUITE_TEST(suite, client_delete);
        ADD_SUITE_TEST(suite, client_list_free);
    }
    return CU_get_error();
}
