#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "HttpRandomGetter.h"
#include "CurlWrapper.h"
#include "MockRandomGetterDep.h"
#include <memory> // unique pointers

class HttpRandomGetterImplTest : public ::testing::Test
{

protected:
	std::unique_ptr<meteringclientlib::detail::HttpRandomGetterImpl> testClass;
	std::string testAccessToken;
	std::string apiEndpoint;
	std::string logLocation;
	std::string outputLocation;
	std::unique_ptr<MockCurlWrapper> mockCurl;
	std::unique_ptr<MockCurlWrapper> mockPublisher;
	// std::unique_ptr<MockReceptionLogger> mockLog;

	// You can do set-up work for each test here.
	HttpRandomGetterImplTest();

	// You can do clean-up work that doesn't throw exceptions here.
	virtual ~HttpRandomGetterImplTest();

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	// Code here will be called immediately after the constructor (right
	// before each test).
	virtual void SetUp();

	// Code here will be called immediately after each test (right
	// before the destructor).
	virtual void TearDown();

	virtual int countAllOccurances(std::string findString, std::string searchString);
};
