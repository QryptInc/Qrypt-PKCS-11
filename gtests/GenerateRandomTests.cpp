#include <thread>

#include "qryptoki_pkcs11_vendor_defs.h"

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
    CK_SESSION_HANDLE session = CK_INVALID_HANDLE;

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    EXPECT_EQ(CKR_CRYPTOKI_NOT_INITIALIZED, C_GenerateRandom(session, data, len));
}

TEST (GenerateRandomTests, SessionNotStarted) {
    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SESSION_HANDLE session = CK_INVALID_HANDLE;

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    EXPECT_EQ(CKR_SESSION_HANDLE_INVALID, C_GenerateRandom(session, data, len));

    EXPECT_TRUE(allZeroes(data, len));

    EXPECT_EQ(CKR_OK, finalize());
}

TEST (GenerateRandomTests, SessionClosed) {
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
    char *stashed_token = setEnvVar(EAAS_TOKEN_ENV_VAR, EMPTY_TOKEN);

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

    revertEnvVar(EAAS_TOKEN_ENV_VAR, stashed_token);
}

TEST (GenerateRandomTests, BogusToken) {
    char *stashed_token = setEnvVar(EAAS_TOKEN_ENV_VAR, BOGUS_TOKEN);

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

    revertEnvVar(EAAS_TOKEN_ENV_VAR, stashed_token);
}

TEST (GenerateRandomTests, BogusBaseHSM) {
    char *stashed_base_hsm = setEnvVar(BASE_HSM_ENV_VAR, BOGUS_PATH);

    EXPECT_EQ(CKR_QRYPT_BASE_HSM_OPEN_FAILED, initializeSingleThreaded());

    EXPECT_EQ(CKR_CRYPTOKI_NOT_INITIALIZED, finalize());

    revertEnvVar(BASE_HSM_ENV_VAR, stashed_base_hsm);
}

TEST (GenerateRandomTests, BogusCACert) {
    char *stashed_ca_cert = setEnvVar(CA_CERT_ENV_VAR, BOGUS_PATH);

    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    CK_SESSION_HANDLE session;
    EXPECT_EQ(CKR_OK, newSession(slotID, session));

    const size_t len = 40;
    CK_BYTE data[len] = {0};

    CK_RV rv = C_GenerateRandom(session, data, len);
    EXPECT_TRUE(rv == CKR_OK || rv == CKR_QRYPT_CA_CERT_FAILURE) << rv;

    if(rv == CKR_OK) {
        EXPECT_FALSE(allZeroes(data, len));
    } else {
        EXPECT_TRUE(allZeroes(data, len));
    }

    EXPECT_EQ(CKR_OK, finalize());
    
    revertEnvVar(CA_CERT_ENV_VAR, stashed_ca_cert);
}

TEST (GenerateRandomTests, ValidRequestBasic) {
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
    const size_t NUM_THREADS = 10;
    std::thread threads[NUM_THREADS];
    CK_RV rvs[NUM_THREADS];

    EXPECT_EQ(CKR_OK, initializeMultiThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    const size_t len = 300;
    CK_BYTE data[NUM_THREADS][len];

    for(size_t i = 0; i < NUM_THREADS; i++) {
        for(size_t j = 0; j < len; j++) {
            data[i][j] = (CK_BYTE)0;
        }
    }

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