#include <string.h>
#include <stdexcept>

#include "common.h"

std::unique_ptr<char[]> setEnvVar(const char *var_name, const char *new_value) {
    char *old_value = getenv(var_name);

    if(old_value == NULL) return NULL;

    size_t old_value_len = strlen(old_value);

    std::unique_ptr<char[]> old_value_copy = std::make_unique<char[]>(old_value_len + 1);
    
    for(size_t i = 0; i < old_value_len + 1; i++)
        old_value_copy[i] = old_value[i];
    
    setenv(var_name, new_value, 1);
    return old_value_copy;
}

void revertEnvVar(const char *var_name, std::unique_ptr<char[]> &stashed_value) {
    if(stashed_value == NULL) {
        unsetenv(var_name);
    } else {
        setenv(var_name, stashed_value.get(), 1);
    }
}

CK_RV initializeSingleThreaded(CK_FUNCTION_LIST_PTR fn_list) {
    return fn_list->C_Initialize(NULL_PTR);
}

CK_RV initializeMultiThreaded(CK_FUNCTION_LIST_PTR fn_list) {
    CK_C_INITIALIZE_ARGS initializeArgs;

    initializeArgs.CreateMutex = NULL;
    initializeArgs.DestroyMutex = NULL;
    initializeArgs.LockMutex = NULL;
    initializeArgs.UnlockMutex = NULL;
    initializeArgs.flags = CKF_OS_LOCKING_OK;
    initializeArgs.pReserved = NULL;

    return fn_list->C_Initialize(&initializeArgs);
}

bool isIntegrationTestSlot(CK_FUNCTION_LIST_PTR fn_list, CK_SLOT_ID slotID) {
    CK_TOKEN_INFO info;
    CK_RV rv = fn_list->C_GetTokenInfo(slotID, &info);

    if(rv != CKR_OK) return false;

    const char *label = (char *)info.label;
    
    const char *integration_label = INTEGRATION_TESTS_TOKEN_LABEL;
    size_t integration_label_len = strlen(integration_label);

    size_t bytes_to_cmp = integration_label_len < 32 ? integration_label_len : 32;

    return memcmp(label, integration_label, integration_label_len) == 0;
}

bool isEmptySlot(CK_FUNCTION_LIST_PTR fn_list, CK_SLOT_ID slotID) {
    CK_TOKEN_INFO info;
    CK_RV rv = fn_list->C_GetTokenInfo(slotID, &info);

    return (rv == CKR_OK) && ((info.flags & CKF_TOKEN_INITIALIZED) == 0);
}

CK_RV newIntegrationTestSlot(CK_FUNCTION_LIST_PTR fn_list, CK_SLOT_ID &slotID) {
    CK_RV rv;

    // slotCount = total # of slots (empty + containing tokens)
    CK_ULONG slotCount;
    rv = fn_list->C_GetSlotList(CK_FALSE, NULL_PTR, &slotCount);
    if(rv != CKR_OK) return rv;

    // slotList = list of all slots
    std::unique_ptr<CK_SLOT_ID[]> slotList = std::make_unique<CK_SLOT_ID[]>(slotCount);
    rv = fn_list->C_GetSlotList(CK_FALSE, slotList.get(), &slotCount);
    if(rv != CKR_OK) return rv;

    // Search for empty slot + try to initalize token
    for(CK_ULONG i = 0; i < slotCount; i++) {
        slotID = slotList[i];

        if(!isEmptySlot(fn_list, slotID)) continue;

        CK_UTF8CHAR_PTR soPin = (CK_UTF8CHAR_PTR)INTEGRATION_TESTS_TOKEN_SO_PIN;
        CK_ULONG soPinLen = strlen(INTEGRATION_TESTS_TOKEN_SO_PIN);
        
        CK_UTF8CHAR_PTR label = (CK_UTF8CHAR_PTR)INTEGRATION_TESTS_TOKEN_LABEL;

        rv = fn_list->C_InitToken(slotID, soPin, soPinLen, label);
        if(rv == CKR_OK) return CKR_OK;
    }

    throw std::runtime_error("No preexisting GTest slots found, and no empty slots could be initialized.");
}

CK_RV getIntegrationTestSlot(CK_FUNCTION_LIST_PTR fn_list, CK_SLOT_ID &slotID) {
    CK_RV rv;

    // slotCount = # of slots containing tokens
    CK_ULONG slotCount;
    rv = fn_list->C_GetSlotList(CK_TRUE, NULL_PTR, &slotCount);
    if(rv != CKR_OK) return rv;

    // slotList = list of slots containing tokens
    std::unique_ptr<CK_SLOT_ID[]> slotList = std::make_unique<CK_SLOT_ID[]>(slotCount);
    rv = fn_list->C_GetSlotList(CK_TRUE, slotList.get(), &slotCount);
    if(rv != CKR_OK) return rv;

    bool foundGTestSlot = false;
    for(CK_ULONG i = 0; i < slotCount; i++) {
        slotID = slotList[i];

        if(!isIntegrationTestSlot(fn_list, slotID)) continue;

        foundGTestSlot = true;
        break;
    }

    if(foundGTestSlot)
        return CKR_OK;
    else
        return newIntegrationTestSlot(fn_list, slotID);
}

CK_RV newSession(CK_FUNCTION_LIST_PTR fn_list,
                 CK_SLOT_ID slotID,
                 CK_SESSION_HANDLE &session) {
    CK_FLAGS flags = CKF_SERIAL_SESSION; // In particular, read-only

    return fn_list->C_OpenSession(slotID, flags, NULL_PTR, NULL_PTR, &session);
}

CK_RV finalize(CK_FUNCTION_LIST_PTR fn_list) {
    return fn_list->C_Finalize(NULL_PTR);
}
