#pragma once

/**
 * This is the header file for the HttpRandomGetter class. This class can
 * not be copied, but can be moved. This class can be used to perform REST
 * requests to get data into a file or a provided buffer.
*/
#include "RandomGetter.h"
#include "ImplPtrTemplate_h.h"
#include <memory> // unique_ptr

namespace meteringclientlib
{
namespace detail
{
class HttpRandomGetterImpl;
} // namespace detail
class HttpRandomGetter : public RandomGetter
{
public:
    static const char *RANDOM_URI;
    // This constant comes from the RPS ICD and is the limit on the random size.
    static const int RANDOM_URI_MAX_SIZE = 512;
    static const int HTTP_FAIL_MAX = RandomGetter::DEFAULT_FAIL_MAX;
    enum class RandomProtocolEnum
    {
        UDP,
        HTTP
    };

public:
    explicit HttpRandomGetter(const char *inputApi, const char *inputToken, const char *inputLogLocation, const char *inputCaPath);
    explicit HttpRandomGetter(const char *inputApi, const char *inputToken, const char *inputLogLocation);

    ~HttpRandomGetter();

    // movable
    HttpRandomGetter(HttpRandomGetter &&rhs) noexcept;
    HttpRandomGetter &operator=(HttpRandomGetter &&rhs) noexcept;

    // and not copyable
    HttpRandomGetter(const HttpRandomGetter &rhs) = delete;
    HttpRandomGetter &operator=(const HttpRandomGetter &rhs) = delete;

    virtual void getRandomFile(uint64_t randomAmount, const char *randomOutputLocation);
    virtual void getRandomBuffer(uint64_t randomAmount, uint8_t *inputBuffer);
    virtual uint64_t getLastWrittenRandAmount();
    // possible to use #include <iosfwd> for declaration of ostream
    // virtual void getRandomOstream(int randomAmount, std::ostream &outputRandomFile);

private:
    ImplPtrTemplate<detail::HttpRandomGetterImpl> hiddenImpPtr;
};
} // namespace meteringclientlib
