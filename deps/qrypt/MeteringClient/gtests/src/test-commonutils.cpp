#include "gmock/gmock.h"
#include "test-commonutils.h"

CommonUtilsTest::CommonUtilsTest()
	: dirName("/tmp/"),
	  fileName("fakename.txt"){};

CommonUtilsTest::~CommonUtilsTest(){};

void CommonUtilsTest::SetUp(){};

void CommonUtilsTest::TearDown(){};

TEST_F(CommonUtilsTest, fileAppendTest)
{
	std::string expectedString = dirName + fileName;
	std::string resultString = meteringclientlib::detail::appendNameIfDir(dirName, fileName.c_str());
	EXPECT_EQ(resultString, expectedString);
}

TEST_F(CommonUtilsTest, windowsFileAppendTest)
{
	dirName = R"(\tmp\)";
	std::string expectedString = dirName + fileName;
	std::string resultString = meteringclientlib::detail::appendNameIfDir(dirName, fileName.c_str());
	EXPECT_EQ(resultString, expectedString);
}

TEST_F(CommonUtilsTest, fileNoAppendTest)
{
	dirName = "/tmp";
	std::string resultString = meteringclientlib::detail::appendNameIfDir(dirName, fileName.c_str());
	EXPECT_EQ(resultString, dirName);
}

TEST_F(CommonUtilsTest, bearerTokenTest)
{
	std::string expectedString = "Authorization: Bearer xyz";
	std::string strToken = "xyz";
	std::string resultString = meteringclientlib::detail::createBearerToken(strToken);
	EXPECT_EQ(resultString, expectedString);
}
