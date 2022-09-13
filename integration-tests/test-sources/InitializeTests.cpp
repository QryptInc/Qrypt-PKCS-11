#include <iostream>

#include "qryptoki_pkcs11_vendor_defs.h"

#include "common.h"
#include "run_tests.h" // runInitializeTests prototype

namespace initializetests {

bool alreadyInitialized(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    rv = initializeSingleThreaded(fn_list);
    return rv == CKR_CRYPTOKI_ALREADY_INITIALIZED;
}

bool emptyBaseHSM(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    std::unique_ptr<char[]> stashed_base_hsm = setEnvVar(BASE_HSM_ENV_VAR, EMPTY_PATH);

    rv = initializeSingleThreaded(fn_list);
    
    revertEnvVar(BASE_HSM_ENV_VAR, stashed_base_hsm);

    return rv == CKR_QRYPT_BASE_HSM_EMPTY;
}

bool bogusBaseHSM(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    std::unique_ptr<char[]> stashed_base_hsm = setEnvVar(BASE_HSM_ENV_VAR, BOGUS_PATH);

    rv = initializeSingleThreaded(fn_list);
    
    revertEnvVar(BASE_HSM_ENV_VAR, stashed_base_hsm);

    return rv == CKR_QRYPT_BASE_HSM_OPEN_FAILED;
}

bool badArgsReservedNonNULL(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    CK_C_INITIALIZE_ARGS initializeArgs;

    initializeArgs.CreateMutex = NULL;
    initializeArgs.DestroyMutex = NULL;
    initializeArgs.LockMutex = NULL;
    initializeArgs.UnlockMutex = NULL;
    initializeArgs.flags = 0;
    initializeArgs.pReserved = &rv; // Any non-NULL pointer will do

    rv = fn_list->C_Initialize(&initializeArgs);
    return rv == CKR_ARGUMENTS_BAD;
}

CK_RV customUnlockMutex(CK_VOID_PTR pMutex) {
    return CKR_OK;
}

bool badArgsSomeNotAllMutexFnsNULL(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    CK_C_INITIALIZE_ARGS initializeArgs;

    initializeArgs.CreateMutex = NULL;
    initializeArgs.DestroyMutex = NULL;
    initializeArgs.LockMutex = NULL;
    initializeArgs.UnlockMutex = &customUnlockMutex;
    initializeArgs.flags = 0;
    initializeArgs.pReserved = NULL;

    rv = fn_list->C_Initialize(&initializeArgs);
    return rv == CKR_ARGUMENTS_BAD;
}

bool validSingleThreaded(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    return rv == CKR_OK;
}

bool validMultiThreaded(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeMultiThreaded(fn_list);
    return rv == CKR_OK;
}

} // initializetests

void runInitializeTests(CK_FUNCTION_LIST_PTR fn_list,
                        size_t &tests_run_acc,
                        size_t &tests_passed_acc) {
    size_t failed = 0;
    size_t passed = 0;

    std::cout << "INITIALIZE" << std::endl;

    if(!initializetests::alreadyInitialized(fn_list)) {
        std::cout << "alreadyInitialized test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!initializetests::emptyBaseHSM(fn_list)) {
        std::cout << "emptyBaseHSM test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!initializetests::bogusBaseHSM(fn_list)) {
        std::cout << "bogusBaseHSM test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!initializetests::badArgsReservedNonNULL(fn_list)) {
        std::cout << "badArgsReservedNonNULL test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!initializetests::badArgsSomeNotAllMutexFnsNULL(fn_list)) {
        std::cout << "badArgsSomeNotAllMutexFnsNULL test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!initializetests::validSingleThreaded(fn_list)) {
        std::cout << "validSingleThreaded test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!initializetests::validMultiThreaded(fn_list)) {
        std::cout << "validMultiThreaded test failed." << std::endl;
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
