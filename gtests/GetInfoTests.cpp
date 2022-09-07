#include <string.h>

#include "gtest/gtest.h"
#include "common.h"

TEST (GetInfoTests, NotInitialized) {
    CK_INFO info;

    CK_RV rv = C_GetInfo(&info);
    EXPECT_EQ(rv, CKR_CRYPTOKI_NOT_INITIALIZED);
}

TEST (GetInfoTests, Valid) {
    CK_INFO info;

    CK_RV rv = initializeSingleThreaded();
    EXPECT_EQ(rv, CKR_OK);

    rv = C_GetInfo(&info);

    EXPECT_EQ(rv, CKR_OK);

    EXPECT_EQ(info.cryptokiVersion.major, 2);
    EXPECT_EQ(info.cryptokiVersion.minor, 40);

    EXPECT_EQ(strncmp("Qrypt, Inc.", (char *)info.manufacturerID, 11), 0);

    char *manufacturerID = (char *)info.manufacturerID;
    manufacturerID[31] = '\0';
    std::cout << "manufacturerID: " << manufacturerID << std::endl;

    EXPECT_EQ(info.flags, 0);

    EXPECT_EQ(strncmp("Wrap of ", (char *)info.libraryDescription, 8), 0);

    char *libraryDescription = (char *)info.libraryDescription;
    libraryDescription[31] = '\0';
    std::cout << "libraryDescription: " << libraryDescription << std::endl;

    EXPECT_EQ(info.libraryVersion.major, 0);
    EXPECT_EQ(info.libraryVersion.minor, 1);

    rv = finalize();
    EXPECT_EQ(rv, CKR_OK);
}