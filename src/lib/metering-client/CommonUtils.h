#pragma once

/**
 * This is the header file for common utility functions for the library.
*/
#include <string> // string

namespace meteringclientlib
{
namespace detail
{
std::string appendNameIfDir(std::string inputPath, const char *inputName);
std::string createBearerToken(std::string strAuthToken);
} // namespace detail
} // namespace meteringclientlib