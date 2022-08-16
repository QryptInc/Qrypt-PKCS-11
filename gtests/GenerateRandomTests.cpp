#include <thread>

#include "gtest/gtest.h"
#include "common.h"

bool allZeroes(CK_BYTE_PTR data, CK_ULONG len) {
    for(CK_ULONG i = 0; i < len; i++) {
        if(data[i] != (CK_BYTE)0)
            return false;
    }
    
    return true;
}

TEST (GenerateRandomTests, NotInitialized) {
    setenv(EAAS_TOKEN_ENV_VAR, VALID_TOKEN, 1);

    CK_SESSION_HANDLE session = CK_INVALID_HANDLE;

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    EXPECT_EQ(CKR_CRYPTOKI_NOT_INITIALIZED, C_GenerateRandom(session, data, len));
}

TEST (GenerateRandomTests, SessionNotStarted) {
    setenv(EAAS_TOKEN_ENV_VAR, VALID_TOKEN, 1);

    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SESSION_HANDLE session = CK_INVALID_HANDLE;

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    EXPECT_EQ(CKR_SESSION_HANDLE_INVALID, C_GenerateRandom(session, data, len));

    EXPECT_TRUE(allZeroes(data, len));

    EXPECT_EQ(CKR_OK, finalize());
}

TEST (GenerateRandomTests, SessionClosed) {
    setenv(EAAS_TOKEN_ENV_VAR, VALID_TOKEN, 1);

    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    CK_SESSION_HANDLE session;
    EXPECT_EQ(CKR_OK, newSession(slotID, session));

    EXPECT_EQ(CKR_OK, C_CloseSession(session));

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    EXPECT_EQ(CKR_SESSION_HANDLE_INVALID, C_GenerateRandom(session, data, len));

    EXPECT_TRUE(allZeroes(data, len));

    EXPECT_EQ(CKR_OK, finalize());
}

TEST (GenerateRandomTests, EmptyToken) {
    setenv(EAAS_TOKEN_ENV_VAR, EMPTY_TOKEN, 1);

    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    CK_SESSION_HANDLE session;
    EXPECT_EQ(CKR_OK, newSession(slotID, session));

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    EXPECT_EQ(CKR_QRYPT_TOKEN_EMPTY, C_GenerateRandom(session, data, len));

    EXPECT_TRUE(allZeroes(data, len));

    EXPECT_EQ(CKR_OK, finalize());
}

TEST (GenerateRandomTests, BogusToken) {
    setenv(EAAS_TOKEN_ENV_VAR, BOGUS_TOKEN, 1);

    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    CK_SESSION_HANDLE session;
    EXPECT_EQ(CKR_OK, newSession(slotID, session));

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    EXPECT_EQ(CKR_QRYPT_TOKEN_INVALID, C_GenerateRandom(session, data, len));

    EXPECT_TRUE(allZeroes(data, len));

    EXPECT_EQ(CKR_OK, finalize());
}

TEST (GenerateRandomTests, BogusBaseHSM) {
    char *base_hsm = getenv(BASE_HSM_ENV_VAR);
    char *base_hsm_copy = NULL;

    if(base_hsm != NULL) {
        size_t base_hsm_len = strlen(base_hsm);

        base_hsm_copy = new char[base_hsm_len + 1];
        strncpy(base_hsm_copy, base_hsm, base_hsm_len + 1);
    }

    setenv(BASE_HSM_ENV_VAR, BOGUS_PATH, 1);
    setenv(EAAS_TOKEN_ENV_VAR, VALID_TOKEN, 1);

    EXPECT_EQ(CKR_QRYPT_BASE_HSM_OPEN_FAILED, initializeSingleThreaded());

    EXPECT_EQ(CKR_CRYPTOKI_NOT_INITIALIZED, finalize());

    if(base_hsm_copy == NULL)
        unsetenv(BASE_HSM_ENV_VAR);
    else {
        setenv(BASE_HSM_ENV_VAR, base_hsm_copy, 1);
        delete[] base_hsm_copy;
    }
}

TEST (GenerateRandomTests, BogusCACert) {
    char *ca_cert = getenv(CA_CERT_ENV_VAR);
    char *ca_cert_copy = NULL;

    if(ca_cert != NULL) {
        size_t ca_cert_len = strlen(ca_cert);

        ca_cert_copy = new char[ca_cert_len + 1];
        strncpy(ca_cert_copy, ca_cert, ca_cert_len + 1);
    }

    setenv(EAAS_TOKEN_ENV_VAR, VALID_TOKEN, 1);
    setenv(CA_CERT_ENV_VAR, BOGUS_PATH, 1);

    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    CK_SESSION_HANDLE session;
    EXPECT_EQ(CKR_OK, newSession(slotID, session));

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    EXPECT_EQ(CKR_QRYPT_CA_CERT_FAILURE, C_GenerateRandom(session, data, len));

    EXPECT_TRUE(allZeroes(data, len));

    EXPECT_EQ(CKR_OK, finalize());
    
    if(ca_cert_copy == NULL)
        unsetenv(CA_CERT_ENV_VAR);
    else {
        setenv(CA_CERT_ENV_VAR, ca_cert_copy, 1);
        delete[] ca_cert_copy;
    }
}

TEST (GenerateRandomTests, ValidRequestBasic) {
    setenv(EAAS_TOKEN_ENV_VAR, VALID_TOKEN, 1);

    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    CK_SESSION_HANDLE session;
    EXPECT_EQ(CKR_OK, newSession(slotID, session));

    const size_t len = 2000;
    CK_BYTE data[len] = {0};
    EXPECT_EQ(CKR_OK, C_GenerateRandom(session, data, len));

    EXPECT_FALSE(allZeroes(data, len));

    EXPECT_EQ(CKR_OK, finalize());
}

TEST (GenerateRandomTests, ValidRequestLengthZero) {
    setenv(EAAS_TOKEN_ENV_VAR, VALID_TOKEN, 1);

    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    CK_SESSION_HANDLE session;
    EXPECT_EQ(CKR_OK, newSession(slotID, session));

    const size_t len = 0;
    CK_BYTE data[len] = {};
    EXPECT_EQ(CKR_OK, C_GenerateRandom(session, data, len));

    EXPECT_EQ(CKR_OK, finalize());
}

void getRandom(CK_SLOT_ID slotID, CK_BYTE_PTR data, CK_ULONG len, CK_RV &rv) {
    CK_SESSION_HANDLE session;

    rv = newSession(slotID, session);
    if(rv != CKR_OK) return;

    rv = C_GenerateRandom(session, data, len);
}

TEST (GenerateRandomTests, ValidRequestManyThreads) {
    setenv(EAAS_TOKEN_ENV_VAR, VALID_TOKEN, 1);

    const size_t NUM_THREADS = 10;
    std::thread threads[NUM_THREADS];
    CK_RV rvs[NUM_THREADS];

    EXPECT_EQ(CKR_OK, initializeMultiThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    const size_t len = 300;
    CK_BYTE data[NUM_THREADS][len];

    for(size_t i = 0; i < NUM_THREADS; i++)
        threads[i] = std::thread(getRandom, slotID, data[i], len, std::ref(rvs[i]));
    
    for(size_t i = 0; i < NUM_THREADS; i++)
        threads[i].join();
    
    for(size_t i = 0; i < NUM_THREADS; i++) {
        EXPECT_EQ(CKR_OK, rvs[i]);
        EXPECT_FALSE(allZeroes(data[i], len));
    }

    EXPECT_EQ(CKR_OK, finalize());
}