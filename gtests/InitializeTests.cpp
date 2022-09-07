#include "qryptoki_pkcs11_vendor_defs.h"

#include "gtest/gtest.h"
#include "common.h"

TEST (InitializeTests, AlreadyInitialized) {
    CK_RV rv = initializeSingleThreaded();
    EXPECT_EQ(rv, CKR_OK);
    
    rv = initializeSingleThreaded();
    EXPECT_EQ(rv, CKR_CRYPTOKI_ALREADY_INITIALIZED);

    rv = finalize();
    EXPECT_EQ(rv, CKR_OK);
}

TEST (InitializeTests, EmptyBaseHSM) {
    char *stashed_base_hsm = setEnvVar(BASE_HSM_ENV_VAR, EMPTY_PATH);

    CK_RV rv = initializeSingleThreaded();
    EXPECT_EQ(rv, CKR_QRYPT_BASE_HSM_EMPTY);

    rv = finalize();
    EXPECT_EQ(rv, CKR_CRYPTOKI_NOT_INITIALIZED);

    revertEnvVar(BASE_HSM_ENV_VAR, stashed_base_hsm);
}

TEST (InitializeTests, BogusBaseHSM) {
    char *stashed_base_hsm = setEnvVar(BASE_HSM_ENV_VAR, BOGUS_PATH);

    CK_RV rv = initializeSingleThreaded();
    EXPECT_EQ(rv, CKR_QRYPT_BASE_HSM_OPEN_FAILED);

    rv = finalize();
    EXPECT_EQ(rv, CKR_CRYPTOKI_NOT_INITIALIZED);

    revertEnvVar(BASE_HSM_ENV_VAR, stashed_base_hsm);
}

TEST (InitializeTests, BadArgsReservedNonNULL) {
    bool *ptr = new bool;

    CK_C_INITIALIZE_ARGS initializeArgs;

    initializeArgs.CreateMutex = NULL;
    initializeArgs.DestroyMutex = NULL;
    initializeArgs.LockMutex = NULL;
    initializeArgs.UnlockMutex = NULL;
    initializeArgs.flags = 0;
    initializeArgs.pReserved = ptr;

    CK_RV rv = C_Initialize(&initializeArgs);
    EXPECT_EQ(rv, CKR_ARGUMENTS_BAD);

    rv = finalize();
    EXPECT_EQ(rv, CKR_CRYPTOKI_NOT_INITIALIZED);

    delete ptr;
}

CK_RV customUnlockMutex(CK_VOID_PTR pMutex) {
    return CKR_OK;
}

TEST (InitializeTests, BadArgsSomeNotAllMutexFnsNULL) {
    CK_C_INITIALIZE_ARGS initializeArgs;

    initializeArgs.CreateMutex = NULL;
    initializeArgs.DestroyMutex = NULL;
    initializeArgs.LockMutex = NULL;
    initializeArgs.UnlockMutex = &customUnlockMutex;
    initializeArgs.flags = 0;
    initializeArgs.pReserved = NULL;

    CK_RV rv = C_Initialize(&initializeArgs);
    EXPECT_EQ(rv, CKR_ARGUMENTS_BAD);

    rv = finalize();
    EXPECT_EQ(rv, CKR_CRYPTOKI_NOT_INITIALIZED);
}

TEST (InitializeTests, ValidSingleThreaded) {
    CK_RV rv = initializeSingleThreaded();
    EXPECT_EQ(rv, CKR_OK);

    rv = finalize();
    EXPECT_EQ(rv, CKR_OK);
}

TEST (InitializeTests, ValidMultiThreaded) {
    CK_RV rv = initializeMultiThreaded();
    EXPECT_EQ(rv, CKR_OK);

    rv = finalize();
    EXPECT_EQ(rv, CKR_OK);
}
