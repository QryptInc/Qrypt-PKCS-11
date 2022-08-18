#include "test-httpgetterimpl.h"
#define NOMINMAX
#include "HttpRandomGetterImpl.h"
#include "base64.h"
#include <string>	// string
#include <memory>	// unique_ptrs
#include <iostream> // ostream, cout
#include <stdio.h>	// remove

HttpRandomGetterImplTest::HttpRandomGetterImplTest()
{
	testAccessToken = ".faketoken";
	testAccessToken = base64_encode(testAccessToken);
	apiEndpoint = "https://notreal";
	logLocation = "./";
	outputLocation = "./";
	mockCurl = std::make_unique<MockCurlWrapper>();
	mockPublisher = std::make_unique<MockCurlWrapper>();
};

HttpRandomGetterImplTest::~HttpRandomGetterImplTest() {};

void HttpRandomGetterImplTest::SetUp(){};

void HttpRandomGetterImplTest::TearDown(){};

int HttpRandomGetterImplTest::countAllOccurances(std::string findString, std::string searchString)
{
	int findCount = 0;
	// Get the first occurrence
	size_t pos = searchString.find(findString);

	// Repeat till end is reached
	while (pos != std::string::npos)
	{
		findCount++;
		// Get the next occurrence from the current position
		pos = searchString.find(findString, pos + findString.size());
	}
	return findCount;
}

TEST_F(HttpRandomGetterImplTest, apiHttpConstructTest)
{
	apiEndpoint = "http://notreal:5000";
	EXPECT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						 apiEndpoint.c_str(), testAccessToken.c_str(), std::move(mockCurl)));
						//   std::move(mockLog), std::move(mockCurl)));
}

TEST_F(HttpRandomGetterImplTest, apiHttpsConstructTest)
{
	apiEndpoint = "https://notreal:5000";
	EXPECT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						 apiEndpoint.c_str(), testAccessToken.c_str(), std::move(mockCurl)));
						//   std::move(mockLog), std::move(mockCurl)));
}

TEST_F(HttpRandomGetterImplTest, apiSlashConstructFailTest)
{
	apiEndpoint = "https://notreal/";
	ASSERT_ANY_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						 apiEndpoint.c_str(), testAccessToken.c_str(), std::move(mockCurl)));
						//   std::move(mockLog), std::move(mockCurl)));
}

TEST_F(HttpRandomGetterImplTest, apiBadConstructFailTest)
{
	apiEndpoint = "notreal";
	ASSERT_ANY_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						 apiEndpoint.c_str(), testAccessToken.c_str(),std::move(mockCurl)));
						//   std::move(mockLog), std::move(mockCurl)));
}

TEST_F(HttpRandomGetterImplTest, mockCurlFailTest)
{

	EXPECT_CALL(*mockCurl, easyPerform())
		.Times(3)
		.WillRepeatedly(::testing::Throw(std::runtime_error("Test error")));
	EXPECT_CALL(*mockCurl, setUrl)
		.Times(3);
	EXPECT_CALL(*mockCurl, setWriteData)
		.Times(3);
	// EXPECT_CALL(*mockLog, logReceptionFile)
	// 	.Times(0);
	ASSERT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						apiEndpoint.c_str(), testAccessToken.c_str(),std::move(mockCurl)));
						//   std::move(mockLog), std::move(mockCurl)));
	std::ostringstream outputStream;
	EXPECT_THROW(testClass->getRandomOstream(1, outputStream), std::runtime_error);
	EXPECT_EQ(testClass->getLastWrittenRandAmount(), 0);
}

TEST_F(HttpRandomGetterImplTest, mockCurlResponseFailTest)
{
	EXPECT_CALL(*mockCurl, easyPerform())
		.Times(3);
	EXPECT_CALL(*mockCurl, setUrl)
		.Times(3);
	EXPECT_CALL(*mockCurl, setWriteData)
		.Times(3);
	EXPECT_CALL(*mockCurl, getResponseCode())
		.Times(3)
		.WillRepeatedly(::testing::Return(400));

	ASSERT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						apiEndpoint.c_str(), testAccessToken.c_str(),std::move(mockCurl)));
	std::ostringstream outputStream;
	EXPECT_THROW(testClass->getRandomOstream(1, outputStream), std::runtime_error);
	EXPECT_EQ(testClass->getLastWrittenRandAmount(), 0);
}

TEST_F(HttpRandomGetterImplTest, mockCurlBadJsonTest)
{
	std::string jsonResponse("boo");
	EXPECT_CALL(*mockCurl, easyPerform())
		.Times(3);
	EXPECT_CALL(*mockCurl, setUrl)
		.Times(3);
	EXPECT_CALL(*mockCurl, setWriteData)
		.Times(3)
		.WillRepeatedly(::testing::SetArgReferee<0>(jsonResponse));
	EXPECT_CALL(*mockCurl, getResponseCode())
		.Times(3)
		.WillRepeatedly(::testing::Return(200));

	ASSERT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						apiEndpoint.c_str(), testAccessToken.c_str(),std::move(mockCurl)));
						//   std::move(mockLog), std::move(mockCurl)));
	std::ostringstream outputStream;
	EXPECT_THROW(testClass->getRandomOstream(1, outputStream), std::runtime_error);
	EXPECT_EQ(testClass->getLastWrittenRandAmount(), 0);
}

TEST_F(HttpRandomGetterImplTest, mockCurlMissingRandomTest)
{
	std::string jsonResponse("{\"test\":2}");
	EXPECT_CALL(*mockCurl, easyPerform())
		.Times(3);
	EXPECT_CALL(*mockCurl, setUrl)
		.Times(3);
	EXPECT_CALL(*mockCurl, setWriteData)
		.Times(3)
		.WillRepeatedly(::testing::SetArgReferee<0>(jsonResponse));
	EXPECT_CALL(*mockCurl, getResponseCode())
		.Times(3)
		.WillRepeatedly(::testing::Return(200));

	ASSERT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						apiEndpoint.c_str(), testAccessToken.c_str(),std::move(mockCurl)));

	std::ostringstream outputStream;
	EXPECT_THROW(testClass->getRandomOstream(1, outputStream), std::runtime_error);
	EXPECT_EQ(testClass->getLastWrittenRandAmount(), 0);
}

TEST_F(HttpRandomGetterImplTest, mockCurlWriteFileRandomNotArrayTest)
{
	std::string jsonResponse("{\"random\":2}");
	EXPECT_CALL(*mockCurl, easyPerform())
		.Times(1);
	EXPECT_CALL(*mockCurl, setUrl)
		.Times(1);
	EXPECT_CALL(*mockCurl, setWriteData)
		.WillOnce(::testing::SetArgReferee<0>(jsonResponse));
	EXPECT_CALL(*mockCurl, getResponseCode())
		.Times(1)
		.WillRepeatedly(::testing::Return(200));

	ASSERT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						apiEndpoint.c_str(), testAccessToken.c_str(),std::move(mockCurl)));
						//   std::move(mockLog), std::move(mockCurl)));
	std::ostringstream outputStream;
	EXPECT_THROW(testClass->getRandomOstream(1, outputStream), std::runtime_error);
	EXPECT_EQ(testClass->getLastWrittenRandAmount(), 0);
}


TEST_F(HttpRandomGetterImplTest, bufferWriteTest)
{
	// each response field is 3 bytes long
	int randomCount = 6;
	std::string jsonResponse("{\"random\":[\"MTIz\",\"NDU2\"]}");
	std::string first_expected_output("123");
	std::string second_expected_output("456");

	uint8_t expectedOutput[6] = {};
	memcpy(&expectedOutput[0], first_expected_output.c_str(), first_expected_output.length());
	memcpy(&expectedOutput[3], second_expected_output.c_str(), second_expected_output.length());
	EXPECT_CALL(*mockCurl, easyPerform())
		.Times(1);
	EXPECT_CALL(*mockCurl, setUrl)
		.Times(1);
	EXPECT_CALL(*mockCurl, setWriteData)
		.Times(1)
		.WillRepeatedly(::testing::SetArgReferee<0>(jsonResponse));
	EXPECT_CALL(*mockCurl, getResponseCode())
		.Times(1)
		.WillRepeatedly(::testing::Return(200));


	ASSERT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						apiEndpoint.c_str(), testAccessToken.c_str(),std::move(mockCurl)));

	uint8_t testBuffer[6] = {};
	EXPECT_NO_THROW(testClass->getRandomBuffer(randomCount, &testBuffer[0]));
	for (int t = 0; t < sizeof(testBuffer); ++t)
	{
		std::cout << testBuffer[t] << " end: " << expectedOutput[t] << std::endl;
	    if (testBuffer[t] != expectedOutput[t])
	    {
	        std::cout << "Mismatch: " << testBuffer[t] << ", " << expectedOutput[t] << ". Index: " << t << std::endl;
	    }
	}
	EXPECT_TRUE(0 == std::memcmp(testBuffer, expectedOutput, sizeof(expectedOutput)));
	EXPECT_EQ(testClass->getLastWrittenRandAmount(), 6);
}

TEST_F(HttpRandomGetterImplTest, bufferWrite2Test)
{
	// each response field is 3 bytes long
	// Expected number of bytes
	int randomCount = 12;
	std::string jsonResponse("{\"random\":[\"MTIz\",\"NDU2\"]}");
	std::string first_expected_output("123");
	std::string second_expected_output("456");
	uint8_t expectedOutput[12] = {};
	memcpy(&expectedOutput[0], first_expected_output.c_str(), first_expected_output.length());
	memcpy(&expectedOutput[3], second_expected_output.c_str(), second_expected_output.length());
	memcpy(&expectedOutput[6], first_expected_output.c_str(), first_expected_output.length());
	memcpy(&expectedOutput[9], second_expected_output.c_str(), second_expected_output.length());

	EXPECT_CALL(*mockCurl, easyPerform())
		.Times(2);
	EXPECT_CALL(*mockCurl, setUrl)
		.Times(2);
	EXPECT_CALL(*mockCurl, setWriteData)
		.Times(2)
		.WillRepeatedly(::testing::SetArgReferee<0>(jsonResponse));
	EXPECT_CALL(*mockCurl, getResponseCode())
		.Times(2)
		.WillRepeatedly(::testing::Return(200));

	ASSERT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						apiEndpoint.c_str(), testAccessToken.c_str(),std::move(mockCurl)));

	uint8_t testBuffer[12] = {};
	EXPECT_NO_THROW(testClass->getRandomBuffer(randomCount, &testBuffer[0]));
	EXPECT_TRUE(0 == std::memcmp(testBuffer, expectedOutput, sizeof(expectedOutput)));
	EXPECT_EQ(testClass->getLastWrittenRandAmount(), 12);
}

TEST_F(HttpRandomGetterImplTest, bufferWrite2ExceptionTest)
{
	// each response field is 3 bytes long
	// Expected number of bytes
	int randomCount = 12;
	std::string jsonResponse("{\"random\":[\"MTIz\",\"NDU2\"]}");
	std::string first_expected_output("123");
	std::string second_expected_output("456");
	uint8_t expectedOutput[12] = {};
	memcpy(&expectedOutput[0], first_expected_output.c_str(), first_expected_output.length());
	memcpy(&expectedOutput[3], second_expected_output.c_str(), second_expected_output.length());

	EXPECT_CALL(*mockCurl, easyPerform())
		.Times(4);
	EXPECT_CALL(*mockCurl, setUrl)
		.Times(4);
	EXPECT_CALL(*mockCurl, setWriteData)
		.Times(4)
		.WillRepeatedly(::testing::SetArgReferee<0>(jsonResponse));
	EXPECT_CALL(*mockCurl, getResponseCode())
		.WillOnce(::testing::Return(200))
		.WillRepeatedly(::testing::Return(400));

	ASSERT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						apiEndpoint.c_str(), testAccessToken.c_str(),std::move(mockCurl)));

	uint8_t testBuffer[12] = {};
	EXPECT_THROW(testClass->getRandomBuffer(randomCount, &testBuffer[0]), std::runtime_error);
	EXPECT_TRUE(0 == std::memcmp(testBuffer, expectedOutput, sizeof(expectedOutput)));
	EXPECT_EQ(testClass->getLastWrittenRandAmount(), 6);
}

TEST_F(HttpRandomGetterImplTest, mockCurlWriteFileRandomTest)
{
	// each response field is 3 bytes long
	std::string jsonResponse("{\"random\":[\"MTIz\",\"NDU2\"]}");
	std::string expectedOutput;
	expectedOutput.append(base64_decode("MTIz"));
	expectedOutput.append(base64_decode("NDU2"));
	EXPECT_CALL(*mockCurl, easyPerform())
		.Times(1);
	EXPECT_CALL(*mockCurl, setUrl)
		.Times(1);
	EXPECT_CALL(*mockCurl, setWriteData)
		.WillOnce(::testing::SetArgReferee<0>(jsonResponse));
	EXPECT_CALL(*mockCurl, getResponseCode())
		.WillOnce(::testing::Return(200));
	ASSERT_NO_THROW(testClass = std::make_unique<meteringclientlib::detail::HttpRandomGetterImpl>(
						apiEndpoint.c_str(), testAccessToken.c_str(),std::move(mockCurl)));
	std::ostringstream outputStream;
	EXPECT_NO_THROW(testClass->getRandomOstream(6, outputStream));
	EXPECT_EQ(outputStream.str(), expectedOutput);
	EXPECT_EQ(testClass->getLastWrittenRandAmount(), 6);
}
