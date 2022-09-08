#include "gtest/gtest.h"
#include "common.h"

TEST (FinalizeTests, NotInitialized) {
    CK_RV rv = finalize();
    EXPECT_EQ(rv, CKR_CRYPTOKI_NOT_INITIALIZED);
}

TEST (FinalizeTests, BadArgs) {
    CK_RV rv = initializeSingleThreaded();
    EXPECT_EQ(rv, CKR_OK);

    bool b = false;
    rv = C_Finalize(&b);
    EXPECT_EQ(rv, CKR_ARGUMENTS_BAD);

    rv = finalize();
    EXPECT_EQ(rv, CKR_OK);
}

TEST (FinalizeTests, DoesActuallyFinalize) {
    CK_RV rv = initializeSingleThreaded();
    EXPECT_EQ(rv, CKR_OK);

    rv = finalize();
    EXPECT_EQ(rv, CKR_OK);

    rv = finalize();
    EXPECT_EQ(rv, CKR_CRYPTOKI_NOT_INITIALIZED);
}