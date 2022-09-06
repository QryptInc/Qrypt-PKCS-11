#include <iostream>

#include "common.h"
#include "run_tests.h"

namespace seedrandomtests {

bool notInitialized(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    CK_SESSION_HANDLE session = CK_INVALID_HANDLE;

    const size_t len = 40;
    CK_BYTE seed[len] = {0};

    rv = fn_list->C_SeedRandom(session, seed, len);
    return rv == CKR_CRYPTOKI_NOT_INITIALIZED;
}

bool sessionNotStarted(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session = CK_INVALID_HANDLE;

    const size_t len = 40;
    CK_BYTE seed[len] = {0};

    rv = fn_list->C_SeedRandom(session, seed, len);
    return rv == CKR_SESSION_HANDLE_INVALID;
}

bool sessionClosed(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;

    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;
    
    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    rv = fn_list->C_CloseSession(session);
    if(rv != CKR_OK) return false;

    const size_t len = 40;
    CK_BYTE seed[len] = {0};
    
    rv = fn_list->C_SeedRandom(session, seed, len);
    return rv == CKR_SESSION_HANDLE_INVALID;
}

bool badArgs(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;

    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;

    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    const size_t len = 40;

    rv = fn_list->C_SeedRandom(session, NULL_PTR, len);
    return rv == CKR_ARGUMENTS_BAD;
}

bool valid(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;

    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;

    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    const size_t len = 40;
    CK_BYTE seed[len] = {0};

    rv = fn_list->C_SeedRandom(session, seed, len);
    return rv == CKR_RANDOM_SEED_NOT_SUPPORTED;
}

} // seedrandomtests

void runSeedRandomTests(CK_FUNCTION_LIST_PTR fn_list,
                        size_t &tests_run_acc,
                        size_t &tests_passed_acc) {
    size_t failed = 0;
    size_t passed = 0;

    std::cout << "SEED RANDOM" << std::endl;

    if(!seedrandomtests::notInitialized(fn_list)) {
        std::cout << "notInitialized test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!seedrandomtests::sessionNotStarted(fn_list)) {
        std::cout << "sessionNotStarted test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!seedrandomtests::sessionClosed(fn_list)) {
        std::cout << "sessionClosed test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!seedrandomtests::badArgs(fn_list)) {
        std::cout << "badArgs test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!seedrandomtests::valid(fn_list)) {
        std::cout << "valid test failed." << std::endl;
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
