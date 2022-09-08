/**
 * This is the header file for the HttpRandomGetterImpl class.
*/

#include "HttpRandomGetter.h"
#define NOMINMAX
#include "ImplPtrTemplate_impl.h"
#include "AccessToken.h"
#include "CurlWrapper.h"
#include "rapidjson/document.h"
#include "CommonUtils.h"
#include <iostream> // ostream, cout
#include <string>   // string
#include <map>      // map
#include <memory>   //shared_ptr, unique
#include <tuple>    // tuple

namespace meteringclientlib
{
namespace detail
{
class HttpRandomGetterImpl
{
public:
private:
    static const char *REQUEST_TYPE;
    static const char *CURL_AGENT;
    static const long BUFFER_SIZE;
    static const long MAX_REDIRS;
    static const long CURL_TCP_KEEPALIVE;
    static const int RETRY_BACKOFF_FIRST_ATTEMPT;
    static const int RETRY_BACKOFF;
    std::unique_ptr<CurlWrapper> curlRpsRest;
    std::string apiEndpoint;
    AccessToken userToken;
    std::string rpsUrlStart;
    std::string randomUri;
    std::string restBuffer;
    ::rapidjson::Document restJson;
    uint64_t lastWrittenRandAmount;

public:
    HttpRandomGetterImpl(const char *inputApi, const char *inputToken, const char *inputLogLocation, const char *inputCaPath)
        : apiEndpoint(inputApi),
          userToken(inputToken),
          rpsUrlStart(),
          restBuffer(),
          lastWrittenRandAmount(0)
    {
        restJson.SetObject();
        checkFqdnFormat(apiEndpoint);
        std::unique_ptr<CurlWrapper> pusherCurl = std::make_unique<CurlWrapper>(REQUEST_TYPE, CURL_AGENT,
                                                                                BUFFER_SIZE, MAX_REDIRS,
                                                                                CURL_TCP_KEEPALIVE, inputCaPath);
        std::string bearerToken = createBearerToken(userToken.getAccessToken());
        pusherCurl->appendSlist("Content-Type: application/json");
        pusherCurl->appendSlist(bearerToken.c_str());
        curlRpsRest = std::make_unique<CurlWrapper>(REQUEST_TYPE, CURL_AGENT,
                                                    BUFFER_SIZE, MAX_REDIRS,
                                                    CURL_TCP_KEEPALIVE, inputCaPath);
        setRandomParams();
        curlRpsRest->appendSlist(bearerToken.c_str());
    }

    HttpRandomGetterImpl(const char *inputApi, const char *inputToken, std::unique_ptr<CurlWrapper> inputCurl)
        : curlRpsRest(std::move(inputCurl)),
          apiEndpoint(inputApi),
          userToken(inputToken),
          rpsUrlStart(),
          restBuffer(),
          lastWrittenRandAmount(0)
    {
        restJson.SetObject();
        checkFqdnFormat(apiEndpoint);
        std::string bearerToken = createBearerToken(userToken.getAccessToken());
        setRandomParams();
        curlRpsRest->appendSlist(bearerToken.c_str());
    }

    ~HttpRandomGetterImpl() = default;

    void getRandomFile(uint64_t randomAmount, const char *randomOutputLocation);
    void getRandomBuffer(uint64_t randomAmount, uint8_t *inputBuffer);
    void getRandomOstream(uint64_t randomAmount, std::ostream &outputRandomFile);
    uint64_t getLastWrittenRandAmount(){return lastWrittenRandAmount;}

private:
    void getRandom(uint64_t randomSize);
    void printProgress(uint64_t randomRecieved, uint64_t totalRandom);
    void checkFqdnFormat(std::string inputFqdn);
    uint64_t writeToOstream(std::ostream &outputRandomFile);
    std::tuple<int, uint64_t> writeToBuffer(uint8_t *inputBuffer, int bufferWritePos);
    static std::size_t writeCallback(char *contents, std::size_t size, std::size_t nmemb, std::string *userp);
    void setRandomParams();
};
} // namespace detail
} // namespace meteringclientlib
