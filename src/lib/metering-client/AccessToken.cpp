/**
 * This is the cpp file for the AccessToken class.
*/

#include "AccessToken.h"
#include "base64.h" // base64 decoding
#include <iostream> // general printing
#include <string>   // string
#include <stdexcept>

namespace meteringclientlib
{
namespace detail
{

// Sets accessToken 
void AccessToken::setAccessToken(const char *inputToken)
{
    accessToken = inputToken;
}

// Sets accessToken 
void AccessToken::setAccessToken(std::string inputToken)
{
    accessToken = inputToken;
}

// Getter function returns accessToken
std::string AccessToken::getAccessToken()
{
    return accessToken;
}

} // namespace detail
} // namespace meteringclientlib
