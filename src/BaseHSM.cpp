#include <stdlib.h>
#include <dlfcn.h>
#include <string>

#include "pkcs11.h"
#include "log.h"
#include "BaseHSM.h"

bool allFunctionsNonNULL(CK_FUNCTION_LIST_PTR list) {
    return list->C_Initialize &&
        list->C_Finalize &&
        list->C_GetInfo &&
        list->C_GetFunctionList &&
        list->C_GetSlotList &&
        list->C_GetSlotInfo &&
        list->C_GetTokenInfo &&
        list->C_WaitForSlotEvent &&
        list->C_GetMechanismList &&
        list->C_GetMechanismInfo &&
        list->C_InitToken &&
        list->C_InitPIN &&
        list->C_SetPIN &&
        list->C_OpenSession &&
        list->C_CloseSession &&
        list->C_CloseAllSessions &&
        list->C_GetSessionInfo &&
        list->C_GetOperationState &&
        list->C_SetOperationState &&
        list->C_Login &&
        list->C_Logout &&
        list->C_CreateObject &&
        list->C_CopyObject &&
        list->C_DestroyObject &&
        list->C_GetObjectSize &&
        list->C_GetAttributeValue &&
        list->C_SetAttributeValue &&
        list->C_FindObjectsInit &&
        list->C_FindObjects &&
        list->C_FindObjectsFinal &&
        list->C_EncryptInit &&
        list->C_Encrypt &&
        list->C_EncryptUpdate &&
        list->C_EncryptFinal &&
        list->C_DecryptInit &&
        list->C_Decrypt &&
        list->C_DecryptUpdate &&
        list->C_DecryptFinal &&
        list->C_DigestInit &&
        list->C_Digest &&
        list->C_DigestUpdate &&
        list->C_DigestKey &&
        list->C_DigestFinal &&
        list->C_SignInit &&
        list->C_Sign &&
        list->C_SignUpdate &&
        list->C_SignFinal &&
        list->C_SignRecoverInit &&
        list->C_SignRecover &&
        list->C_VerifyInit &&
        list->C_Verify &&
        list->C_VerifyUpdate &&
        list->C_VerifyFinal &&
        list->C_VerifyRecoverInit &&
        list->C_VerifyRecover &&
        list->C_DigestEncryptUpdate &&
        list->C_DecryptDigestUpdate &&
        list->C_SignEncryptUpdate &&
        list->C_DecryptVerifyUpdate &&
        list->C_GenerateKey &&
        list->C_GenerateKeyPair &&
        list->C_WrapKey &&
        list->C_UnwrapKey &&
        list->C_DeriveKey &&
        list->C_SeedRandom &&
        list->C_GenerateRandom &&
        list->C_GetFunctionStatus &&
        list->C_CancelFunction;
}

CK_RV BaseHSM::initialize() {
    char *path = getenv("QRYPT_BASE_HSM_PATH");
    if(path == NULL || path[0] == '\0') {
		ERROR_MSG("Environment variable QRYPT_BASE_HSM_PATH empty.");
		return CKR_QRYPT_BASE_HSM_EMPTY;
	}

    void *handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL | RTLD_DEEPBIND);
    if(handle == NULL) {
        ERROR_MSG("Could not find base HSM at path given by QRYPT_BASE_HSM_PATH.");
        return CKR_QRYPT_BASE_HSM_OPEN_FAILED;
    }

    CK_C_GetFunctionList Base_C_GetFunctionList;
    Base_C_GetFunctionList = (CK_C_GetFunctionList)dlsym(handle, "C_GetFunctionList");
    if(Base_C_GetFunctionList == NULL) {
        ERROR_MSG("Could not find base HSM's C_GetFunctionList.");
        return CKR_QRYPT_BASE_HSM_OPEN_FAILED;
    }

    CK_FUNCTION_LIST_PTR list;

    CK_RV rv = (*Base_C_GetFunctionList)(&list);
    if(rv != CKR_OK) {
        ERROR_MSG("Base HSM's C_GetFunctionList returned non-OK value: %lu.", rv);
        return CKR_QRYPT_BASE_HSM_OPEN_FAILED;
    } else if(list == NULL) {
        ERROR_MSG("Base HSM's function list is NULL.");
        return CKR_QRYPT_BASE_HSM_OPEN_FAILED;
    } else if(!allFunctionsNonNULL(list)) {
        ERROR_MSG("Base HSM's function list incomplete.");
        return CKR_QRYPT_BASE_HSM_OPEN_FAILED;
    }

    this->baseFunctionList = list;
    return CKR_OK;
}

void *BaseHSM::getFunction(std::string fn_name) {
    if(fn_name == "C_Initialize")
        return (void *)this->baseFunctionList->C_Initialize;
    else if(fn_name == "C_Finalize")
        return (void *)this->baseFunctionList->C_Finalize;
    else if(fn_name == "C_GetInfo")
        return (void *)this->baseFunctionList->C_GetInfo;
    else if(fn_name == "C_GetFunctionList")
        return (void *)this->baseFunctionList->C_GetFunctionList;
    else if(fn_name == "C_GetSlotList")
        return (void *)this->baseFunctionList->C_GetSlotList;
    else if(fn_name == "C_GetSlotInfo")
        return (void *)this->baseFunctionList->C_GetSlotInfo;
    else if(fn_name == "C_GetTokenInfo")
        return (void *)this->baseFunctionList->C_GetTokenInfo;
    else if(fn_name == "C_WaitForSlotEvent")
        return (void *)this->baseFunctionList->C_WaitForSlotEvent;
    else if(fn_name == "C_GetMechanismList")
        return (void *)this->baseFunctionList->C_GetMechanismList;
    else if(fn_name == "C_GetMechanismInfo")
        return (void *)this->baseFunctionList->C_GetMechanismInfo;
    else if(fn_name == "C_InitToken")
        return (void *)this->baseFunctionList->C_InitToken;
    else if(fn_name == "C_InitPIN")
        return (void *)this->baseFunctionList->C_InitPIN;
    else if(fn_name == "C_SetPIN")
        return (void *)this->baseFunctionList->C_SetPIN;
    else if(fn_name == "C_OpenSession")
        return (void *)this->baseFunctionList->C_OpenSession;
    else if(fn_name == "C_CloseSession")
        return (void *)this->baseFunctionList->C_CloseSession;
    else if(fn_name == "C_CloseAllSessions")
        return (void *)this->baseFunctionList->C_CloseAllSessions;
    else if(fn_name == "C_GetSessionInfo")
        return (void *)this->baseFunctionList->C_GetSessionInfo;
    else if(fn_name == "C_GetOperationState")
        return (void *)this->baseFunctionList->C_GetOperationState;
    else if(fn_name == "C_SetOperationState")
        return (void *)this->baseFunctionList->C_SetOperationState;
    else if(fn_name == "C_Login")
        return (void *)this->baseFunctionList->C_Login;
    else if(fn_name == "C_Logout")
        return (void *)this->baseFunctionList->C_Logout;
    else if(fn_name == "C_CreateObject")
        return (void *)this->baseFunctionList->C_CreateObject;
    else if(fn_name == "C_CopyObject")
        return (void *)this->baseFunctionList->C_CopyObject;
    else if(fn_name == "C_DestroyObject")
        return (void *)this->baseFunctionList->C_DestroyObject;
    else if(fn_name == "C_GetObjectSize")
        return (void *)this->baseFunctionList->C_GetObjectSize;
    else if(fn_name == "C_GetAttributeValue")
        return (void *)this->baseFunctionList->C_GetAttributeValue;
    else if(fn_name == "C_SetAttributeValue")
        return (void *)this->baseFunctionList->C_SetAttributeValue;
    else if(fn_name == "C_FindObjectsInit")
        return (void *)this->baseFunctionList->C_FindObjectsInit;
    else if(fn_name == "C_FindObjects")
        return (void *)this->baseFunctionList->C_FindObjects;
    else if(fn_name == "C_FindObjectsFinal")
        return (void *)this->baseFunctionList->C_FindObjectsFinal;
    else if(fn_name == "C_EncryptInit")
        return (void *)this->baseFunctionList->C_EncryptInit;
    else if(fn_name == "C_Encrypt")
        return (void *)this->baseFunctionList->C_Encrypt;
    else if(fn_name == "C_EncryptUpdate")
        return (void *)this->baseFunctionList->C_EncryptUpdate;
    else if(fn_name == "C_EncryptFinal")
        return (void *)this->baseFunctionList->C_EncryptFinal;
    else if(fn_name == "C_DecryptInit")
        return (void *)this->baseFunctionList->C_DecryptInit;
    else if(fn_name == "C_Decrypt")
        return (void *)this->baseFunctionList->C_Decrypt;
    else if(fn_name == "C_DecryptUpdate")
        return (void *)this->baseFunctionList->C_DecryptUpdate;
    else if(fn_name == "C_DecryptFinal")
        return (void *)this->baseFunctionList->C_DecryptFinal;
    else if(fn_name == "C_DigestInit")
        return (void *)this->baseFunctionList->C_DigestInit;
    else if(fn_name == "C_Digest")
        return (void *)this->baseFunctionList->C_Digest;
    else if(fn_name == "C_DigestUpdate")
        return (void *)this->baseFunctionList->C_DigestUpdate;
    else if(fn_name == "C_DigestKey")
        return (void *)this->baseFunctionList->C_DigestKey;
    else if(fn_name == "C_DigestFinal")
        return (void *)this->baseFunctionList->C_DigestFinal;
    else if(fn_name == "C_SignInit")
        return (void *)this->baseFunctionList->C_SignInit;
    else if(fn_name == "C_Sign")
        return (void *)this->baseFunctionList->C_Sign;
    else if(fn_name == "C_SignUpdate")
        return (void *)this->baseFunctionList->C_SignUpdate;
    else if(fn_name == "C_SignFinal")
        return (void *)this->baseFunctionList->C_SignFinal;
    else if(fn_name == "C_SignRecoverInit")
        return (void *)this->baseFunctionList->C_SignRecoverInit;
    else if(fn_name == "C_SignRecover")
        return (void *)this->baseFunctionList->C_SignRecover;
    else if(fn_name == "C_VerifyInit")
        return (void *)this->baseFunctionList->C_VerifyInit;
    else if(fn_name == "C_Verify")
        return (void *)this->baseFunctionList->C_Verify;
    else if(fn_name == "C_VerifyUpdate")
        return (void *)this->baseFunctionList->C_VerifyUpdate;
    else if(fn_name == "C_VerifyFinal")
        return (void *)this->baseFunctionList->C_VerifyFinal;
    else if(fn_name == "C_VerifyRecoverInit")
        return (void *)this->baseFunctionList->C_VerifyRecoverInit;
    else if(fn_name == "C_VerifyRecover")
        return (void *)this->baseFunctionList->C_VerifyRecover;
    else if(fn_name == "C_DigestEncryptUpdate")
        return (void *)this->baseFunctionList->C_DigestEncryptUpdate;
    else if(fn_name == "C_DecryptDigestUpdate")
        return (void *)this->baseFunctionList->C_DecryptDigestUpdate;
    else if(fn_name == "C_SignEncryptUpdate")
        return (void *)this->baseFunctionList->C_SignEncryptUpdate;
    else if(fn_name == "C_DecryptVerifyUpdate")
        return (void *)this->baseFunctionList->C_DecryptVerifyUpdate;
    else if(fn_name == "C_GenerateKey")
        return (void *)this->baseFunctionList->C_GenerateKey;
    else if(fn_name == "C_GenerateKeyPair")
        return (void *)this->baseFunctionList->C_GenerateKeyPair;
    else if(fn_name == "C_WrapKey")
        return (void *)this->baseFunctionList->C_WrapKey;
    else if(fn_name == "C_UnwrapKey")
        return (void *)this->baseFunctionList->C_UnwrapKey;
    else if(fn_name == "C_DeriveKey")
        return (void *)this->baseFunctionList->C_DeriveKey;
    else if(fn_name == "C_SeedRandom")
        return (void *)this->baseFunctionList->C_SeedRandom;
    else if(fn_name == "C_GenerateRandom")
        return (void *)this->baseFunctionList->C_GenerateRandom;
    else if(fn_name == "C_GetFunctionStatus")
        return (void *)this->baseFunctionList->C_GetFunctionStatus;
    else if(fn_name == "C_CancelFunction")
        return (void *)this->baseFunctionList->C_CancelFunction;
    
    return NULL;
}