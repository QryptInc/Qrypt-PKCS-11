#include "gtest/gtest.h"
#include "common.h"

TEST (GetFunctionListTests, BadArgs) {
    CK_RV rv = C_GetFunctionList(NULL_PTR);
    EXPECT_EQ(rv, CKR_ARGUMENTS_BAD);
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

TEST (GetFunctionListTests, Valid) {
    CK_FUNCTION_LIST_PTR pFunctionList = NULL;

    CK_RV rv = C_GetFunctionList(&pFunctionList);
    EXPECT_EQ(rv, CKR_OK);

    ASSERT_TRUE(pFunctionList != NULL);
    EXPECT_TRUE(allFunctionsNonNULL(pFunctionList));
}

