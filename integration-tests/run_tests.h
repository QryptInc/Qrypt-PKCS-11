#include <stdlib.h>

#include "cryptoki.h"

/**
 * Each of these functions runs integration tests for one
 * of the 6 overwritten PKCS#11 functions. It adds the number
 * of tests run to tests_run_acc and the number of tests passed
 * to tests_passed_acc.
 */

void runInitializeTests(CK_FUNCTION_LIST_PTR fn_list,
                        size_t &tests_run_acc,
                        size_t &tests_passed_acc);

void runFinalizeTests(CK_FUNCTION_LIST_PTR fn_list,
                      size_t &tests_run_acc,
                      size_t &tests_passed_acc);

void runGetInfoTests(CK_FUNCTION_LIST_PTR fn_list,
                     size_t &tests_run_acc,
                     size_t &tests_passed_acc);

void runGetFunctionListTests(CK_FUNCTION_LIST_PTR fn_list,
                             size_t &tests_run_acc,
                             size_t &tests_passed_acc);

void runSeedRandomTests(CK_FUNCTION_LIST_PTR fn_list,
                        size_t &tests_run_acc,
                        size_t &tests_passed_acc);

void runGenerateRandomTests(CK_FUNCTION_LIST_PTR fn_list,
                            size_t &tests_run_acc,
                            size_t &tests_passed_acc);