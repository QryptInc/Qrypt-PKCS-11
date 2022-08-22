/**
 * This is the cpp file for the CommonUtils.h.
*/
#include "CommonUtils.h"
#include <regex>  // regex
#include <string> // string

namespace meteringclientlib
{
namespace detail
{
std::string appendNameIfDir(std::string inputPath, const char *inputName)
{
    std::string returnString = inputPath;
    if (std::regex_match(returnString, std::regex(".*[\\\\/]$")))
    {
        returnString += inputName;
    }

    return returnString;
}

/**
 * Simple function to create bearer token for HTTP header
*/
std::string createBearerToken(std::string strAuthToken)
{
    std::string bearerToken = "Authorization: Bearer " + strAuthToken;
    return bearerToken;
}

} // namespace detail
} // namespace meteringclientlib