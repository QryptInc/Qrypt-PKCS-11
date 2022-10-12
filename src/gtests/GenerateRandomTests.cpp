#include <thread>       /* std::thread */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <set>          /* std::set */

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
    std::unique_ptr<char[]> stashed_token = setEnvVar(EAAS_TOKEN_ENV_VAR, EMPTY_TOKEN);

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
    std::unique_ptr<char[]> stashed_token = setEnvVar(EAAS_TOKEN_ENV_VAR, BOGUS_TOKEN);

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

TEST (GenerateRandomTests, ExpiredToken) {
    std::unique_ptr<char[]> stashed_token = setEnvVar(EAAS_TOKEN_ENV_VAR, EXPIRED_TOKEN);

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

TEST (GenerateRandomTests, BogusCACert) {
    std::unique_ptr<char[]> stashed_ca_cert = setEnvVar(CA_CERT_ENV_VAR, BOGUS_PATH);

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

TEST (GenerateRandomTests, BadArgs) {
    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    CK_SESSION_HANDLE session;
    EXPECT_EQ(CKR_OK, newSession(slotID, session));

    const size_t len = 2000;
    EXPECT_EQ(CKR_ARGUMENTS_BAD, C_GenerateRandom(session, NULL_PTR, len));

    EXPECT_EQ(CKR_OK, finalize());
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

/**
 * This test will fail with small probability due to the birthday paradox.
 * There are 2^32 possible uint32_t's, and we assume that ~6400 randomly chosen
 * uint32_t's will all be distinct. This is probably true, because 6400 << 2^(32/2)
 */
TEST (GenerateRandomTests, NoReuseManyThreads) {
    srand(time(NULL));

    const size_t NUM_THREADS = 100;
    const size_t MAX_REQUEST_SIZE_IN_BYTES = 256;
    std::thread threads[NUM_THREADS];
    CK_RV rvs[NUM_THREADS];

    const size_t BYTES_PER_32_BITS = 4;

    EXPECT_EQ(CKR_OK, initializeMultiThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    size_t request_sizes_in_bytes[NUM_THREADS];
    CK_BYTE data[NUM_THREADS][MAX_REQUEST_SIZE_IN_BYTES];

    for(size_t i = 0; i < NUM_THREADS; i++) {
        size_t size_in_32_bits = (rand() % MAX_REQUEST_SIZE_IN_BYTES) / BYTES_PER_32_BITS;
        request_sizes_in_bytes[i] = BYTES_PER_32_BITS * size_in_32_bits;

        for(size_t j = 0; j < MAX_REQUEST_SIZE_IN_BYTES; j++) {
            data[i][j] = (CK_BYTE)0;
        }
    }

    for(size_t i = 0; i < NUM_THREADS; i++)
        threads[i] = std::thread(getRandom, slotID, data[i], request_sizes_in_bytes[i], std::ref(rvs[i]));
    
    for(size_t i = 0; i < NUM_THREADS; i++)
        threads[i].join();
    
    for(size_t i = 0; i < NUM_THREADS; i++) {
        EXPECT_EQ(CKR_OK, rvs[i]);
        EXPECT_TRUE(request_sizes_in_bytes[i] == 0 || !allZeroes(data[i], request_sizes_in_bytes[i]));
    }

    EXPECT_EQ(CKR_OK, finalize());

    std::set<uint32_t> seen;

    for(size_t i = 0; i < NUM_THREADS; i++) {
        uint32_t *data_in_32_bits = (uint32_t *)data[i];

        for(size_t j = 0; j < request_sizes_in_bytes[i] / BYTES_PER_32_BITS; j++) {
            EXPECT_EQ(seen.count(data_in_32_bits[j]), 0);
            seen.insert(data_in_32_bits[j]);
        }
    }
}
