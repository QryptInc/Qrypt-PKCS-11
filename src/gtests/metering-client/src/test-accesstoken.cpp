#include "gmock/gmock.h"
#include "test-accesstoken.h"
#include "base64.h"

AccessTokenTest::AccessTokenTest()
{
	testAccessToken = ".faketoken";
	testAccessToken = base64_encode(testAccessToken);
};

AccessTokenTest::~AccessTokenTest(){};

void AccessTokenTest::SetUp(){};

void AccessTokenTest::TearDown(){};

TEST_F(AccessTokenTest, stringConstructTest)
{
	meteringclientlib::detail::AccessToken userToken(testAccessToken);
	EXPECT_EQ(testAccessToken, userToken.getAccessToken());
	
}

TEST_F(AccessTokenTest, charConstructTest)
{
	meteringclientlib::detail::AccessToken userToken(testAccessToken.c_str());
	EXPECT_EQ(testAccessToken, userToken.getAccessToken());
}

TEST_F(AccessTokenTest, stringSetTest)
{
	meteringclientlib::detail::AccessToken userToken(testAccessToken);
	testAccessToken = ".faketoken2";
	testAccessToken = base64_encode(testAccessToken);
	userToken.setAccessToken(testAccessToken);
	EXPECT_EQ(testAccessToken, userToken.getAccessToken());
	
}

TEST_F(AccessTokenTest, charSetTest)
{
	meteringclientlib::detail::AccessToken userToken(testAccessToken);
	testAccessToken = ".faketoken2";
	testAccessToken = base64_encode(testAccessToken);
	userToken.setAccessToken(testAccessToken.c_str());
	EXPECT_EQ(testAccessToken, userToken.getAccessToken());
	
}
