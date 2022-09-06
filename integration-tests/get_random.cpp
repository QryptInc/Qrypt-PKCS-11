#include <iostream>
#include <dlfcn.h>
#include <memory>
#include <cstring>

#include "cryptoki.h"

#ifndef LIBRARY_PATH
    #error LIBRARY_PATH not set
#endif

void *dl_handle = NULL;
CK_FUNCTION_LIST_PTR functionList = NULL;

using namespace std;

string cryptokiFailErrorMsg(string fn_name, CK_RV rv) {
    return fn_name + " failed. rv = " + to_string(rv);
}

void setHandleAndFunctionList() {
    dl_handle = dlopen(LIBRARY_PATH, RTLD_LAZY);

    if(dl_handle == NULL) {
        const char *error_msg = dlerror();
        
        if(error_msg == NULL) {
            throw runtime_error("dlopen returned NULL.");
        } else {
            throw runtime_error("dlopen failed: " + std::string(error_msg));
        }
    }
    
    CK_C_GetFunctionList C_GetFunctionList;
    *(void **)&C_GetFunctionList = dlsym(dl_handle, "C_GetFunctionList");

    if(C_GetFunctionList == NULL)
        throw runtime_error("dlsym returned NULL.");
    
    CK_RV rv = (*C_GetFunctionList)(&functionList);
    
    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_GetFunctionList", rv));
    
    if(functionList == NULL)
        throw runtime_error("functionList is NULL even though C_GetFunctionList OK");
}

void initialize() {
    CK_RV rv = functionList->C_Initialize(NULL_PTR);

    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_Initialize", rv));
}

bool isEmptySlot(CK_SLOT_ID slotID) {
    CK_TOKEN_INFO info;
    CK_RV rv = functionList->C_GetTokenInfo(slotID, &info);

    return (rv == CKR_OK) && ((info.flags & CKF_TOKEN_INITIALIZED) == 0);
}

bool isEnd2EndSlot(CK_SLOT_ID slotID) {
    CK_TOKEN_INFO info;
    CK_RV rv = functionList->C_GetTokenInfo(slotID, &info);

    char *label = (char *)info.label;
    return strncmp(label, "End-2-End Example Token         ", 32) == 0;
}

CK_SLOT_ID newEnd2EndSlot() {
    CK_SLOT_ID slotID;
    CK_RV rv;

    // slotCount = total # of slots (empty + containing tokens)
    CK_ULONG slotCount;
    rv = functionList->C_GetSlotList(CK_FALSE, NULL_PTR, &slotCount);
    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_GetSlotList", rv));
    
    // slotList = list of all slots
    unique_ptr<CK_SLOT_ID[]> slotList = make_unique<CK_SLOT_ID[]>(slotCount);

    rv = functionList->C_GetSlotList(CK_TRUE, (CK_SLOT_ID_PTR)slotList.get(), &slotCount);
    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_GetSlotList", rv));
    
    for(CK_ULONG i = 0; i < slotCount; i++) {
        slotID = slotList[i];

        if(!isEmptySlot(slotID)) continue;

        CK_UTF8CHAR_PTR soPin = (CK_UTF8CHAR_PTR)"end2endso";
        CK_ULONG soPinLen = strlen((char *)soPin);

        CK_UTF8CHAR_PTR label = (CK_UTF8CHAR_PTR)"End-2-End Example Token         ";
        if(strlen((char *)label) != 32)
            throw runtime_error("label must have length 32");
        
        rv = functionList->C_InitToken(slotID, soPin, soPinLen, label);
        if(rv == CKR_OK) return slotID;

        throw runtime_error(cryptokiFailErrorMsg("C_InitToken", rv));
    }

    throw runtime_error("No existing E2E token and no empty slots.");
}

CK_SLOT_ID getEnd2EndSlot() {
    CK_SLOT_ID slotID;
    CK_RV rv;

    // slotCount = # of slots containing tokens
    CK_ULONG slotCount;
    rv = functionList->C_GetSlotList(CK_TRUE, NULL_PTR, &slotCount);
    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_GetSlotList", rv));

    // slotList = list of slots containing tokens
    unique_ptr<CK_SLOT_ID[]> slotList = make_unique<CK_SLOT_ID[]>(slotCount);

    rv = functionList->C_GetSlotList(CK_TRUE, (CK_SLOT_ID_PTR)slotList.get(), &slotCount);
    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_GetSlotList", rv));

    for(CK_ULONG i = 0; i < slotCount; i++) {
        slotID = slotList[i];

        if(isEnd2EndSlot(slotID)) return slotID;
    }

    return newEnd2EndSlot();
}

CK_SESSION_HANDLE openSession(CK_SLOT_ID slotID) {
    CK_SESSION_HANDLE session;

    CK_FLAGS sessionFlags = CKF_SERIAL_SESSION; // In particular, read-only.
    CK_NOTIFY myNotify = NULL_PTR; // For now. Should read up on callback.

    CK_RV rv = functionList->C_OpenSession(slotID, sessionFlags, NULL_PTR, NULL_PTR, &session);

    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_OpenSession", rv));

    return session;
}

void generateRandom(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG len) {
    CK_RV rv = functionList->C_GenerateRandom(session, data, len);

    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_GenerateRandom", rv));
}

void finalize() {
    if(functionList == NULL) return;

    CK_RV rv = functionList->C_Finalize(NULL_PTR);

    if(rv != CKR_OK)
        cout << cryptokiFailErrorMsg("C_Finalize", rv) << endl;
}

void cleanup() {
    finalize();
    if(dl_handle != NULL) dlclose(dl_handle);
}

int main() {
    CK_SLOT_ID slotID;
    CK_SESSION_HANDLE session;

    try {
        setHandleAndFunctionList();
        initialize();

        slotID = getEnd2EndSlot();
        session = openSession(slotID);

        CK_ULONG len;

        cout << "How many bytes do you want? (Enter a number from 0 to 104858. WARNING: This will count towards your Qrypt monthly usage.) ";
        cin >> len;

        if(len < 0 || len > 104858) throw runtime_error("Invalid number of bytes.");

        unique_ptr<CK_BYTE[]> data = make_unique<CK_BYTE[]>(len);

        generateRandom(session, data.get(), len);

        CK_ULONG len_to_print = len < 10 ? len : 10;

        cout << "Here are the first few bytes:" << endl;

        for(CK_ULONG i = 0; i < len_to_print; i++) {
            cout << "data[" << i << "] = " << (unsigned int)data[i] << endl;
        }
    }
    catch (runtime_error &ex) {
        cerr << ex.what() << endl;

        cleanup();
        return 1;
    }

    cleanup();
    return 0;
}
