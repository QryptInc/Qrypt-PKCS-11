/**
 * Contains all the mock objects for RandomGetter and derived classes.
 */
#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "CurlWrapper.h"
#include "HttpRandomGetter.h"

class MockCurlWrapper : public meteringclientlib::detail::CurlWrapper
{
public:
    //error in gmock forces no usage of mock_method with clang
    MOCK_METHOD1(setUrl, void(const char *));
    // function pointers don't mock well...
    MOCK_METHOD1(setWriteData, void(std::string &));
    MOCK_METHOD0(easyPerform, CURLcode());
    MOCK_METHOD0(getResponseCode, long());
    MockCurlWrapper()
        : CurlWrapper() {}
    virtual ~MockCurlWrapper() {}
};
