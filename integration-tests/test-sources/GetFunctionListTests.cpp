#include <iostream>

#include "common.h"
#include "run_tests.h" // runGetFunctionListTests prototype

namespace getfunctionlisttests {

bool badArgs(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv = fn_list->C_GetFunctionList(NULL_PTR);
    return rv == CKR_ARGUMENTS_BAD;
}

bool allFunctionsNonNULL(CK_FUNCTION_LIST_PTR pFunctionList) {
    return pFunctionList->C_Initialize &&
        pFunctionList->C_Finalize &&
        pFunctionList->C_GetInfo &&
        pFunctionList->C_GetFunctionList &&
        pFunctionList->C_GetSlotList &&
        pFunctionList->C_GetSlotInfo &&
        pFunctionList->C_GetTokenInfo &&
        pFunctionList->C_WaitForSlotEvent &&
        pFunctionList->C_GetMechanismList &&
        pFunctionList->C_GetMechanismInfo &&
        pFunctionList->C_InitToken &&
        pFunctionList->C_InitPIN &&
        pFunctionList->C_SetPIN &&
        pFunctionList->C_OpenSession &&
        pFunctionList->C_CloseSession &&
        pFunctionList->C_CloseAllSessions &&
        pFunctionList->C_GetSessionInfo &&
        pFunctionList->C_GetOperationState &&
        pFunctionList->C_SetOperationState &&
        pFunctionList->C_Login &&
        pFunctionList->C_Logout &&
        pFunctionList->C_CreateObject &&
        pFunctionList->C_CopyObject &&
        pFunctionList->C_DestroyObject &&
        pFunctionList->C_GetObjectSize &&
        pFunctionList->C_GetAttributeValue &&
        pFunctionList->C_SetAttributeValue &&
        pFunctionList->C_FindObjectsInit &&
        pFunctionList->C_FindObjects &&
        pFunctionList->C_FindObjectsFinal &&
        pFunctionList->C_EncryptInit &&
        pFunctionList->C_Encrypt &&
        pFunctionList->C_EncryptUpdate &&
        pFunctionList->C_EncryptFinal &&
        pFunctionList->C_DecryptInit &&
        pFunctionList->C_Decrypt &&
        pFunctionList->C_DecryptUpdate &&
        pFunctionList->C_DecryptFinal &&
        pFunctionList->C_DigestInit &&
        pFunctionList->C_Digest &&
        pFunctionList->C_DigestUpdate &&
        pFunctionList->C_DigestKey &&
        pFunctionList->C_DigestFinal &&
        pFunctionList->C_SignInit &&
        pFunctionList->C_Sign &&
        pFunctionList->C_SignUpdate &&
        pFunctionList->C_SignFinal &&
        pFunctionList->C_SignRecoverInit &&
        pFunctionList->C_SignRecover &&
        pFunctionList->C_VerifyInit &&
        pFunctionList->C_Verify &&
        pFunctionList->C_VerifyUpdate &&
        pFunctionList->C_VerifyFinal &&
        pFunctionList->C_VerifyRecoverInit &&
        pFunctionList->C_VerifyRecover &&
        pFunctionList->C_DigestEncryptUpdate &&
        pFunctionList->C_DecryptDigestUpdate &&
        pFunctionList->C_SignEncryptUpdate &&
        pFunctionList->C_DecryptVerifyUpdate &&
        pFunctionList->C_GenerateKey &&
        pFunctionList->C_GenerateKeyPair &&
        pFunctionList->C_WrapKey &&
        pFunctionList->C_UnwrapKey &&
        pFunctionList->C_DeriveKey &&
        pFunctionList->C_SeedRandom &&
        pFunctionList->C_GenerateRandom &&
        pFunctionList->C_GetFunctionStatus &&
        pFunctionList->C_CancelFunction;
}

bool valid(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;

    CK_FUNCTION_LIST_PTR pFunctionList;
    rv = fn_list->C_GetFunctionList(&pFunctionList);

    return (rv == CKR_OK)
        && (pFunctionList != NULL_PTR)
        && (allFunctionsNonNULL(pFunctionList));
}

} // getfunctionlisttests

void runGetFunctionListTests(CK_FUNCTION_LIST_PTR fn_list,
                             size_t &tests_run_acc,
                             size_t &tests_passed_acc) {
    size_t failed = 0;
    size_t passed = 0;

    std::cout << "GET FUNCTION LIST" << std::endl;

    if(!getfunctionlisttests::badArgs(fn_list)) {
        std::cout << "badArgs test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!getfunctionlisttests::valid(fn_list)) {
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
