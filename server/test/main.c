#include <stdio.h>
#include <stdlib.h>
#include <CUnit/Basic.h>

#include "test_utils.h"
#include "test_client.h"
#include "test_config.h"
#include "test_logging.h"
#include "test_server.h"
#include "test_smtp_cmd_utils.h"
#include "test_smtp_cmd.h"

int main(void) {
    if (CU_initialize_registry() != CUE_SUCCESS) {
        printf("%s\n",CU_get_error_msg());
        exit(CU_get_error());
    }

    CU_pSuite client_suite = CUnitCreateSuite("client_suite");
    CU_pSuite config_suite = CUnitCreateSuite("config_suite");
    CU_pSuite logging_suite = CUnitCreateSuite("logging_suite");
    CU_pSuite server_suite = CUnitCreateSuite("server_suite");
    CU_pSuite smtp_cmd_utils_suite = CUnitCreateSuite("smtp_cmd_utils_suite");
    CU_pSuite smtp_cmd_suite = CUnitCreateSuite("smtp_cmd_suite");
    if (client_suite && config_suite && logging_suite
        && server_suite && smtp_cmd_utils_suite && smtp_cmd_suite) {
        add_to_suit_client_test(client_suite);
        add_to_suit_config_test(config_suite);
        add_to_suit_logging_test(logging_suite);
        add_to_suit_server_test(server_suite);
        add_to_suit_smtp_cmd_utils_test(smtp_cmd_utils_suite);
        add_to_suit_smtp_cmd_test(smtp_cmd_suite);
    }
    else {
        CU_cleanup_registry();
        printf("%s\n",CU_get_error_msg());
        exit(CU_get_error());
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    printf("\n");
    CU_basic_show_failures(CU_get_failure_list());
    printf("\n");
    CU_cleanup_registry();
    return CU_get_error();
}
