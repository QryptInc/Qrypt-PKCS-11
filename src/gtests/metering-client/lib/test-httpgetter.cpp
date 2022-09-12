#include "test-httpgetter.h"
#include "HttpRandomGetter.h"
#include "base64.h"
#include <string> // string
#include <memory> // unique_ptrs
#include <vector>

HttpRandomGetterTest::HttpRandomGetterTest()
{
	testAccessToken = ".faketoken";
	testAccessToken = base64_encode(testAccessToken);
	apiEndpoint = "https://notreal";
	// randomFormat = meteringclientlib::HttpRandomGetter::RandomHttpFormatEnum::hex;
	// randomPrecision = 64;
	logLocation = ".";
	outputLocation = ".";
	caPath = "";
};

HttpRandomGetterTest::~HttpRandomGetterTest(){};

void HttpRandomGetterTest::SetUp(){};

void HttpRandomGetterTest::TearDown(){};

TEST_F(HttpRandomGetterTest, apiSlashConstructFailTest)
{
	apiEndpoint = "https://notreal/";
	ASSERT_ANY_THROW(meteringclientlib::HttpRandomGetter getterObj(
						 apiEndpoint.c_str(), testAccessToken.c_str(), logLocation.c_str()););
	ASSERT_ANY_THROW(meteringclientlib::HttpRandomGetter getterObj(
						 apiEndpoint.c_str(), testAccessToken.c_str(), logLocation.c_str(), caPath.c_str()););
}

TEST_F(HttpRandomGetterTest, apiFormatConstructFailTest)
{
	apiEndpoint = "htt:notreal";
	ASSERT_ANY_THROW(meteringclientlib::HttpRandomGetter getterObj(
						 apiEndpoint.c_str(), testAccessToken.c_str(), logLocation.c_str()););
	ASSERT_ANY_THROW(meteringclientlib::HttpRandomGetter getterObj(
						 apiEndpoint.c_str(), testAccessToken.c_str(), logLocation.c_str(), caPath.c_str()););
}
