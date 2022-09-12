#include <iostream>
#include <dlfcn.h>

#include "run_tests.h"

#ifndef QRYPTOKI_PATH
    #error QRYPTOKI_PATH macro not defined.
#endif

int main() {
    // DYNAMICALLY LINK QRYPTOKI

    void *dl_handle = dlopen(QRYPTOKI_PATH, RTLD_LAZY);

    if(dl_handle == NULL) {
        std::cout << dlerror() << std::endl;
        return 1;
    }

    // GET FUNCTION LIST

    CK_C_GetFunctionList C_GetFunctionList;
    *(void **)&C_GetFunctionList = dlsym(dl_handle, "C_GetFunctionList");

    if(C_GetFunctionList == NULL) {
        std::cout << dlerror() << std::endl;
        dlclose(dl_handle);
        return 1;
    }

    CK_FUNCTION_LIST_PTR fn_list = NULL;
    CK_RV rv = (*C_GetFunctionList)(&fn_list);

    if(rv != CKR_OK) {
        std::cout << "C_GetFunctionList not OK, rv = "  << rv << std::endl;
        dlclose(dl_handle);
        return 1;
    } else if(fn_list == NULL) {
        std::cout << "fn_list is NULL even though C_GetFunctionList OK" << std::endl;
        dlclose(dl_handle);
        return 1;
    }

    // RUN ALL TESTS

    size_t tests_run_acc = 0;
    size_t tests_passed_acc = 0;

    runInitializeTests(fn_list, tests_run_acc, tests_passed_acc);
    runFinalizeTests(fn_list, tests_run_acc, tests_passed_acc);
    runGetInfoTests(fn_list, tests_run_acc, tests_passed_acc);
    runGetFunctionListTests(fn_list, tests_run_acc, tests_passed_acc);
    runSeedRandomTests(fn_list, tests_run_acc, tests_passed_acc);
    runGenerateRandomTests(fn_list, tests_run_acc, tests_passed_acc);

    std::cout << "OVERALL RESULTS" << std::endl;

    std::cout << "Tests run: " << tests_run_acc << std::endl;
    std::cout << "Tests passed: " << tests_passed_acc << std::endl;

    dlclose(dl_handle);
    return 0;
}