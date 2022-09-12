#pragma once

#include "gtest/gtest.h"
#include "HttpRandomGetter.h"
#include <string> // string

class HttpRandomGetterTest : public ::testing::Test
{

protected:
	std::unique_ptr<meteringclientlib::HttpRandomGetter> testClass;
	std::string testAccessToken;
	std::string apiEndpoint;
	std::string logLocation;
	std::string outputLocation;
	std::string caPath;

	// You can do set-up work for each test here.
	HttpRandomGetterTest();

	// You can do clean-up work that doesn't throw exceptions here.
	virtual ~HttpRandomGetterTest();

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	// Code here will be called immediately after the constructor (right
	// before each test).
	virtual void SetUp();

	// Code here will be called immediately after each test (right
	// before the destructor).
	virtual void TearDown();
};
