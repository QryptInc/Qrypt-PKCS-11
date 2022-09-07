#pragma once

/**
 * This is the header file for the AccessToken class. This class stores and helps retrieve the auth token.
*/

#include <string> //string

namespace meteringclientlib
{
namespace detail
{
class AccessToken
{
public:
private:
    std::string accessToken;

public:
    AccessToken(const char *inputAccessToken)
        : accessToken()
    {
        setAccessToken(inputAccessToken);
    }
    AccessToken(std::string inputAccessToken)
        : accessToken()
    {
        setAccessToken(inputAccessToken);
    }

    ~AccessToken() = default;

    void setAccessToken(const char *inputToken);
    void setAccessToken(std::string inputToken);
    std::string getAccessToken();
};
} // namespace detail
} // namespace meteringclientlib