#include "gtest/gtest.h"
#include "RandomGetter.h"
#include <string> //string

TEST(VersionTest, getVersionTest)
{
    std::string expectedVersionInfo("1.2.1");
    std::string versionInfo(meteringclientlib::MeteringClientLibraryVersion());
    EXPECT_EQ(expectedVersionInfo, versionInfo);
}