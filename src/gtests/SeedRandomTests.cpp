#include "gtest/gtest.h"
#include "common.h"

TEST (SeedRandomTests, NotInitialized) {
    CK_SESSION_HANDLE session = CK_INVALID_HANDLE;

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    EXPECT_EQ(CKR_CRYPTOKI_NOT_INITIALIZED, C_SeedRandom(session, data, len));
}

TEST (SeedRandomTests, SessionNotStarted) {
    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SESSION_HANDLE session = CK_INVALID_HANDLE;

    const size_t len = 40;
    CK_BYTE seed[len] = {0};
    EXPECT_EQ(CKR_SESSION_HANDLE_INVALID, C_SeedRandom(session, seed, len));

    EXPECT_EQ(CKR_OK, finalize());
}

TEST (SeedRandomTests, SessionClosed) {
    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    CK_SESSION_HANDLE session;
    EXPECT_EQ(CKR_OK, newSession(slotID, session));

    EXPECT_EQ(CKR_OK, C_CloseSession(session));

    const size_t len = 40;
    CK_BYTE seed[len] = {0};
    EXPECT_EQ(CKR_SESSION_HANDLE_INVALID, C_GenerateRandom(session, seed, len));

    EXPECT_EQ(CKR_OK, finalize());
}

TEST (SeedRandomTests, Valid) {
    EXPECT_EQ(CKR_OK, initializeSingleThreaded());

    CK_SLOT_ID slotID;
    EXPECT_EQ(CKR_OK, getGTestSlot(slotID));

    CK_SESSION_HANDLE session;
    EXPECT_EQ(CKR_OK, newSession(slotID, session));

    const size_t len = 40;
    CK_BYTE seed[len] = {0};
    EXPECT_EQ(CKR_RANDOM_SEED_NOT_SUPPORTED, C_SeedRandom(session, seed, len));

    EXPECT_EQ(CKR_OK, finalize());
}
