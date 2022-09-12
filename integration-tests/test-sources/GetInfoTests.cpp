#include <iostream>

#include "common.h"
#include "run_tests.h"

namespace getinfotests {

bool notInitialized(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    CK_INFO info;

    rv = fn_list->C_GetInfo(&info);
    return rv == CKR_CRYPTOKI_NOT_INITIALIZED;
}

bool badArgs(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    rv = fn_list->C_GetInfo(NULL_PTR);
    return rv == CKR_ARGUMENTS_BAD;
}

bool valid(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_INFO info;

    rv = fn_list->C_GetInfo(&info);
    if(rv != CKR_OK) return false;

    int major = (int)info.cryptokiVersion.major;
    int minor = (int)info.cryptokiVersion.minor;

    std::cout << "Cryptoki v" << major << "." << minor << std::endl;

    char *manufacturerID = (char *)info.manufacturerID;
    manufacturerID[31] = '\0';
    std::cout << "manufacturerID: " << manufacturerID << std::endl;

    std::cout << "flags: " << info.flags << std::endl;

    char *libraryDescription = (char *)info.libraryDescription;
    libraryDescription[31] = '\0';
    std::cout << "libraryDescription: " << libraryDescription << std::endl;

    major = info.libraryVersion.major;
    minor = info.libraryVersion.minor;

    std::cout << "Library v" << major << "." << minor << std::endl << std::endl;

    return true;
}

} // getinfotests

void runGetInfoTests(CK_FUNCTION_LIST_PTR fn_list,
                     size_t &tests_run_acc,
                     size_t &tests_passed_acc) {
    size_t failed = 0;
    size_t passed = 0;

    std::cout << "GET INFO" << std::endl;

    if(!getinfotests::notInitialized(fn_list)) {
        std::cout << "notInitialized test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!getinfotests::badArgs(fn_list)) {
        std::cout << "badArgs test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!getinfotests::valid(fn_list)) {
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
