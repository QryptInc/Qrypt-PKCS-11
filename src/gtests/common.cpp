#include <string.h>
#include <stdexcept>

#include "common.h"

char *setEnvVar(const char *var_name, const char *new_value) {
    char *old_value = getenv(var_name);

    if(old_value == NULL) return NULL;

    size_t old_value_len = strlen(old_value);
    char *old_value_copy = new char[old_value_len + 1];
    
    for(size_t i = 0; i < old_value_len + 1; i++)
        old_value_copy[i] = old_value[i];
    
    setenv(var_name, new_value, 1);
    return old_value_copy;
}

void revertEnvVar(const char *var_name, char *stashed_value) {
    if(stashed_value == NULL) {
        unsetenv(var_name);
    } else {
        setenv(var_name, stashed_value, 1);
        delete[] stashed_value;
    }
}

CK_RV initializeSingleThreaded() {
    return C_Initialize(NULL_PTR);
}

CK_RV initializeMultiThreaded() {
    CK_C_INITIALIZE_ARGS initializeArgs;

    initializeArgs.CreateMutex = NULL;
    initializeArgs.DestroyMutex = NULL;
    initializeArgs.LockMutex = NULL;
    initializeArgs.UnlockMutex = NULL;
    initializeArgs.flags = CKF_OS_LOCKING_OK;
    initializeArgs.pReserved = NULL;

    return C_Initialize(&initializeArgs);
}

bool isGTestSlot(CK_SLOT_ID slotID) {
    CK_TOKEN_INFO info;
    CK_RV rv = C_GetTokenInfo(slotID, &info);

    if(rv != CKR_OK) return false;

    char *label = (char *)info.label;
    return strncmp(label, GTEST_TOKEN_LABEL, GTEST_TOKEN_LABEL_LEN) == 0;
}

bool isEmptySlot(CK_SLOT_ID slotID) {
    CK_TOKEN_INFO info;
    CK_RV rv = C_GetTokenInfo(slotID, &info);

    return (rv == CKR_OK) && ((info.flags & CKF_TOKEN_INITIALIZED) == 0);
}

CK_RV newGTestSlot(CK_SLOT_ID &slotID) {
    CK_RV rv;

    // slotCount = total # of slots (empty + containing tokens)
    CK_ULONG slotCount;
    rv = C_GetSlotList(CK_FALSE, NULL_PTR, &slotCount);
    if(rv != CKR_OK) return rv;

    // slotList = list of all slots
    CK_SLOT_ID_PTR slotList = new CK_SLOT_ID[slotCount];
    rv = C_GetSlotList(CK_FALSE, slotList, &slotCount);
    if(rv != CKR_OK) return rv;

    // Search for empty slot + try to initalize token
    for(CK_ULONG i = 0; i < slotCount; i++) {
        slotID = slotList[i];

        if(!isEmptySlot(slotID)) continue;

        CK_UTF8CHAR_PTR soPin = (CK_UTF8CHAR_PTR)GTEST_TOKEN_SO_PIN;
        CK_ULONG soPinLen = (CK_ULONG)GTEST_TOKEN_SO_PIN_LEN;
        
        CK_UTF8CHAR_PTR label = (CK_UTF8CHAR_PTR)GTEST_TOKEN_LABEL;

        rv = C_InitToken(slotID, soPin, soPinLen, label);
        if(rv == CKR_OK) {
            delete[] slotList;
            return CKR_OK;
        }
    }

    delete[] slotList;
    throw std::runtime_error("No preexisting GTest slots found, and no empty slots could be initialized.");
}

CK_RV getGTestSlot(CK_SLOT_ID &slotID) {
    CK_RV rv;

    // slotCount = # of slots containing tokens
    CK_ULONG slotCount;
    rv = C_GetSlotList(CK_TRUE, NULL_PTR, &slotCount);
    if(rv != CKR_OK) return rv;

    // slotList = list of slots containing tokens
    CK_SLOT_ID_PTR slotList = new CK_SLOT_ID[slotCount];
    rv = C_GetSlotList(CK_TRUE, slotList, &slotCount);
    if(rv != CKR_OK) return rv;

    bool foundGTestSlot = false;
    for(CK_ULONG i = 0; i < slotCount; i++) {
        slotID = slotList[i];

        if(!isGTestSlot(slotID)) continue;

        foundGTestSlot = true;
        break;
    }

    delete[] slotList;

    if(foundGTestSlot)
        return CKR_OK;
    else
        return newGTestSlot(slotID);
}

CK_RV newSession(CK_SLOT_ID slotID, CK_SESSION_HANDLE &session) {
    CK_FLAGS flags = CKF_SERIAL_SESSION; // In particular, read-only

    return C_OpenSession(slotID, flags, NULL_PTR, NULL_PTR, &session);
}

CK_RV finalize() {
    return C_Finalize(NULL_PTR);
}
