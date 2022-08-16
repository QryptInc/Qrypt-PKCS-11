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
    char *base_hsm = getenv(BASE_HSM_ENV_VAR);
    char *base_hsm_copy = NULL;

    if(base_hsm != NULL) {
        size_t base_hsm_len = strlen(base_hsm);

        base_hsm_copy = new char[base_hsm_len + 1];
        strncpy(base_hsm_copy, base_hsm, base_hsm_len + 1);
    }

    setenv(BASE_HSM_ENV_VAR, EMPTY_PATH, 1);
    setenv(EAAS_TOKEN_ENV_VAR, VALID_TOKEN, 1);

    CK_RV rv = initializeSingleThreaded();
    EXPECT_EQ(rv, CKR_QRYPT_BASE_HSM_EMPTY);

    rv = finalize();
    EXPECT_EQ(rv, CKR_CRYPTOKI_NOT_INITIALIZED);

    if(base_hsm_copy == NULL)
        unsetenv(BASE_HSM_ENV_VAR);
    else {
        setenv(BASE_HSM_ENV_VAR, base_hsm_copy, 1);
        delete[] base_hsm_copy;
    }
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
