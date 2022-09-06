#include <iostream>

#include "common.h"
#include "run_tests.h" // runFinalizeTests prototype

namespace finalizetests {

bool notInitialized(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = finalize(fn_list);
    return rv == CKR_CRYPTOKI_NOT_INITIALIZED;
}

bool badArgs(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    bool b = false; // Anything will do

    rv = fn_list->C_Finalize(&b);
    return rv == CKR_ARGUMENTS_BAD;
}

bool doesActuallyFinalize(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    rv = finalize(fn_list);
    if(rv != CKR_OK) return false;

    rv = finalize(fn_list);
    return rv == CKR_CRYPTOKI_NOT_INITIALIZED;
}

} // finalizetests

void runFinalizeTests(CK_FUNCTION_LIST_PTR fn_list,
                      size_t &tests_run_acc,
                      size_t &tests_passed_acc) {
    size_t failed = 0;
    size_t passed = 0;

    std::cout << "FINALIZE" << std::endl;

    if(!finalizetests::notInitialized(fn_list)) {
        std::cout << "notInitialized test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!finalizetests::badArgs(fn_list)) {
        std::cout << "badArgs test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!finalizetests::doesActuallyFinalize(fn_list)) {
        std::cout << "doesActuallyFinalize test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    std::cout << "Tests run: " << (passed + failed) << std::endl;
    std::cout << "Tests passed: " << passed << std::endl << std::endl;

    tests_run_acc += (passed + failed);
    tests_passed_acc += passed;

    finalize(fn_list);
}
