#include <iostream>
#include <dlfcn.h>

#include "pkcs11.h"

const CK_SLOT_ID SLOT = 1653064145;
const char *LIBRARY_PATH = "/workspaces/qryptoki/build/src/libqryptoki.so";

void *dl_handle = NULL;

using namespace std;

void setDLHandle() {
    dl_handle = dlopen(LIBRARY_PATH, RTLD_LAZY);

    if(dl_handle == NULL)
        throw runtime_error("dlopen returned NULL.");
}

void getFunction(void **ptr_to_fn, string fn_name) {
    *ptr_to_fn = dlsym(dl_handle, fn_name.c_str());

    if(*ptr_to_fn == NULL)
        throw runtime_error(fn_name + " not found.");
}

string cryptokiFailErrorMsg(string fn_name, CK_RV rv) {
    return fn_name + " failed. rv = " + to_string(rv);
}

void initialize() {
    CK_C_Initialize C_Initialize;
    getFunction((void **)&C_Initialize, "C_Initialize");

    CK_RV rv = (*C_Initialize)(NULL_PTR);

    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_Initialize", rv));
}

CK_SESSION_HANDLE openSession(CK_SLOT_ID slotID) {
    CK_SESSION_HANDLE session;

    CK_C_OpenSession C_OpenSession;
    getFunction((void **)&C_OpenSession, "C_OpenSession");

    CK_FLAGS sessionFlags = CKF_SERIAL_SESSION; // In particular, read-only.
    CK_NOTIFY myNotify = NULL_PTR; // For now. Should read up on callback.

    CK_RV rv = (*C_OpenSession)(slotID, sessionFlags, NULL_PTR, NULL_PTR, &session);

    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_OpenSession", rv));

    return session;
}

void login(CK_SESSION_HANDLE session) {
    CK_C_Login C_Login;
    getFunction((void **)&C_Login, "C_Login");

    const size_t userPinLen = 4;
    CK_UTF8CHAR userPin[userPinLen] = {'u', 's', 'e', 'r'};

    CK_RV rv = (*C_Login)(session, CKU_USER, userPin, userPinLen);

    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_Login", rv));
}

void generateRandom(CK_SESSION_HANDLE session, CK_BYTE_PTR data, CK_ULONG len) {
    CK_C_GenerateRandom C_GenerateRandom;
    getFunction((void **)&C_GenerateRandom, "C_GenerateRandom");

    CK_RV rv = (*C_GenerateRandom)(session, data, len);

    if(rv != CKR_OK)
        throw runtime_error(cryptokiFailErrorMsg("C_GenerateRandom", rv));
}

void finalize() {
    CK_C_Finalize C_Finalize;

    try {
        getFunction((void **)&C_Finalize, "C_Finalize");
    } catch (...) {
        return;
    }

    CK_RV rv = (*C_Finalize)(NULL_PTR);

    if(rv != CKR_OK)
        cout << cryptokiFailErrorMsg("C_Finalize", rv) << endl;
}

void cleanup() {
    finalize();
    if(dl_handle != NULL) dlclose(dl_handle);
}

int main() {
    CK_SESSION_HANDLE session;

    try {
        setDLHandle();
        initialize();
        session = openSession(SLOT);
        login(session);

        const CK_ULONG len = 1024;
        CK_BYTE data[len];

        generateRandom(session, data, len);

        for(CK_ULONG i = 0; i < 10; i++) {
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
