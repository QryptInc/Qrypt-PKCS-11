/**
 * This is the cpp file for the HttpRandomGetter and HttpRandomGetterImpl class.
 */

#include "HttpRandomGetter.h"
#include "HttpRandomGetterImpl.h"
#include "ImplPtrTemplate_impl.h"
#include "AccessToken.h"
#include "CurlWrapper.h"
#include "CommonUtils.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"
#include <fstream>     // ofstream
#include <iostream>    // ostream, cout
#include <string>      // string
#include <bitset>      // bitset
#include <exception>   // exceptions
#include <curl/curl.h> // curl functions
#include <algorithm>   // min
#include <regex>       // regex
#include <map>         // map
#include <memory>      // unique
#include <tuple>       // tuple
#include <chrono>      // seconds
#include <thread>      // sleep_for
#include "base64.h"

namespace meteringclientlib
{
namespace detail
{
// const map of allowed random formats for a http random getter
// These constants were derivied from a sample curl call.
const char *HttpRandomGetterImpl::REQUEST_TYPE = "GET";
const char *HttpRandomGetterImpl::CURL_AGENT = "curl/7.63.0";
const long HttpRandomGetterImpl::BUFFER_SIZE = 102400L;
const long HttpRandomGetterImpl::MAX_REDIRS = 50L;
const long HttpRandomGetterImpl::CURL_TCP_KEEPALIVE = 1L;
const int HttpRandomGetterImpl::RETRY_BACKOFF_FIRST_ATTEMPT = 2;
const int HttpRandomGetterImpl::RETRY_BACKOFF = 10;

/**
 * getRandom(int) sets the size option in the randomUri string and performs the call to the
 * load balancer via the CurlWrapper object and parses the return into a JSON object with
 * rapidjson. On call, it will clear out the buffers holding random data in the string buffer
 * and the rapidjson object.
 *
 * Programmer beware, this must be externally gated by RANDOM_URI_MAX_SIZE.
 * Do not call directly without having called setRandomParams before.
 */
void HttpRandomGetterImpl::getRandom(uint64_t randomSize)
{
    if (randomSize > HttpRandomGetter::RANDOM_URI_MAX_SIZE)
    {
        throw std::runtime_error("Broke random getter limit per call");
    }

    randomUri = rpsUrlStart;
    randomUri += "?size=";
    randomUri += std::to_string(randomSize);

    for (int attemptCount = 1; attemptCount <= HttpRandomGetter::HTTP_FAIL_MAX; ++attemptCount)
    {

        // Clear out all the buffers before use.
        restBuffer.clear();
        rapidjson::Document().Swap(restJson);

        curlRpsRest->setUrl(randomUri.c_str());
        curlRpsRest->setWriteFunc(writeCallback);
        curlRpsRest->setWriteData(restBuffer);
        curlRpsRest->setCaInfo();
        CURLcode retCurl;
        try
        {
            retCurl = curlRpsRest->easyPerform();
        }
        catch (const std::exception &e)
        {
            if (attemptCount < HttpRandomGetter::HTTP_FAIL_MAX)
            {
                continue;
            }
            else
            {
                throw;
            }
        }

        long response_code = curlRpsRest->getResponseCode();
        curlRpsRest->resetOpt();
        if (response_code != 200)
        {
            if (attemptCount < HttpRandomGetter::HTTP_FAIL_MAX)
            {
                if (response_code == 429)
                {
                    // Only do time back off for 429 which is rate limiting
                    if(attemptCount == 1) {
                        std::this_thread::sleep_for(std::chrono::seconds(RETRY_BACKOFF_FIRST_ATTEMPT));
                    }
                    else if(attemptCount >= 2){
                        std::this_thread::sleep_for(std::chrono::seconds(RETRY_BACKOFF));
                    }
                }
                continue;
            }
            else
            {
                throw std::runtime_error("Reponse code: " + std::to_string(response_code));
            }
        }

        restJson.Parse(restBuffer.c_str());

        if (restJson.HasParseError())
        {
            if (attemptCount < HttpRandomGetter::HTTP_FAIL_MAX)
            {
                continue;
            }
            else
            {
                throw std::runtime_error(::rapidjson::GetParseError_En(restJson.GetParseError()));
            }
        }
        else if (!restJson.IsObject())
        {
            if (attemptCount < HttpRandomGetter::HTTP_FAIL_MAX)
            {
                continue;
            }
            else
            {
                throw std::runtime_error("JSON document is not an object.");
            }
        }

        if (!restJson.HasMember("random"))
        {
            if (attemptCount < HttpRandomGetter::HTTP_FAIL_MAX)
            {
                continue;
            }
            else
            {
                throw std::runtime_error("Missing random in REST response.");
            }
        }
        break;
    }
}

/**
 *  checkFqdnFormat does a quick regex check to make sure the
 * input fqdn starts with 'https://' or 'http://' and does not end with '/'
 */
void HttpRandomGetterImpl::checkFqdnFormat(std::string inputFqdn)
{
    if (!(std::regex_match(inputFqdn, std::regex("^http[s]?://.+[^/]$"))))
    {
        throw std::logic_error("Incorrect API format");
    }
}

/*
 * writeToOstream should be called after a getRandom. This function will write whatever
 * is in the rapidjson object into the passed in ostream reference.
 * This function will output the values to a binary file after performing a base64 decode operation.
 */
uint64_t HttpRandomGetterImpl::writeToOstream(std::ostream &outputRandomFile)
{
    // add checks to make sure getRandom was already called
    // assume max amount is 512KB which is 524288 which falls under the uint64_t max value.
    // if the max amount changes, the data type or logic must change for keeping track
    // of random amount.
    uint64_t randomSize = 0;
    const ::rapidjson::Value &randomArray = restJson["random"];
    // some assumptions of how random is stuffed into the JSON based on RPS ICD
    if (!randomArray.IsArray())
    {
        throw std::runtime_error("Random in JSON is not an array.");
    }

    for (::rapidjson::SizeType arrayIndex = 0; arrayIndex < randomArray.Size(); ++arrayIndex)
    {
        auto &m = randomArray[arrayIndex];
        std::string decodedRandom = base64_decode(m.GetString());
        outputRandomFile.write(decodedRandom.c_str(), decodedRandom.length());
        randomSize += decodedRandom.length();
    }

    return randomSize;
}

/*
 * writeToBuffer should be called after a getRandom. This function is gated to only
 * deal with a binary output. This function pushes all the random in the rapid json
 * object into the buffer pointer. The passed in buffer position will be moved to
 * the end of all the data pushed into the buffer and returned.
 * Programmer beware, this function will blindly trust the passed in buffer pointer
 * and buffer write position.
 */
std::tuple<int, uint64_t> HttpRandomGetterImpl::writeToBuffer(uint8_t *inputBuffer, int bufferWritePos)
{
    // add checks to make sure getRandom was already called
    const ::rapidjson::Value &randomArray = restJson["random"];
    // assume max amount is 512KB which is 524288 which falls under the int max value.
    // if the max amount changes, the data type or logic must change for keeping track
    // of random size.
    uint64_t randomSize = 0;
    // some assumptions of how random is stuffed into the JSON based on RPS ICD
    if (!randomArray.IsArray())
    {
        throw std::runtime_error("Random in JSON is not an array.");
    }
    for (::rapidjson::SizeType arrayIndex = 0; arrayIndex < randomArray.Size(); ++arrayIndex)
    {
        auto &m = randomArray[arrayIndex];
        std::string decodedRandom = base64_decode(m.GetString());
        memcpy(inputBuffer + bufferWritePos, decodedRandom.c_str(), decodedRandom.length());
        bufferWritePos += decodedRandom.length();
        randomSize += decodedRandom.length();
    }

    return std::make_tuple(bufferWritePos, randomSize);
}

/**
 * getRandomFile(int) will open a file stream to the output random
 * file and calls getRandomOstream to fill that file.
 */
void HttpRandomGetterImpl::getRandomFile(uint64_t randomAmount, const char *randomOutputLocation)
{
    std::string outputFile(randomOutputLocation);
    outputFile = appendNameIfDir(outputFile, RandomGetter::RANDOM_FILE_NAME);

    std::ofstream outputRandomFile;
    outputRandomFile.open(outputFile, std::ofstream::app | std::ofstream::binary);

    if (!outputRandomFile.is_open())
    {
        std::string exceptionMessage("Unable to open random output ");
        exceptionMessage += outputFile;
        exceptionMessage += " file";
        throw std::runtime_error(exceptionMessage);
    }

    getRandomOstream(randomAmount, outputRandomFile);
    // file stream are closed when object is destroyed.
}

/**
 * getRandomBuffer(int) will get the passed in value for bytes of random.
 * This call will break it down to legal random API calls and push the random
 * data in the set format to the buffer pointer.
 * This function only support a binary output currently. The buffer size must
 * be amount * 1024 bytes. Each random is held in an unsigned long long.
 * Programmer beware, the buffer pointer must have enough allocated memory to
 * hold all the random data.
 */
void HttpRandomGetterImpl::getRandomBuffer(uint64_t randomAmount, uint8_t *inputBuffer)
{
    int bufferWritePos = 0;
    lastWrittenRandAmount = 0;
    const char *fake_metadata = "No meta data";
    uint64_t randomCount = randomAmount;
    while (randomCount > 0)
    {
        uint64_t dataToGet = (std::min)(((randomCount + static_cast<uint64_t>(meteringclientlib::B_TO_KB) - 1) / static_cast<uint64_t>(meteringclientlib::B_TO_KB)),
                                        static_cast<uint64_t>(HttpRandomGetter::RANDOM_URI_MAX_SIZE));
        // try to get dataToGet amount
        getRandom(dataToGet);
        // write returns actual dataToGet amount made available
        std::tuple<int, uint64_t> writeReturn = writeToBuffer(inputBuffer, bufferWritePos);
        bufferWritePos = std::get<0>(writeReturn);
        uint64_t amountWritten = std::get<1>(writeReturn);
        lastWrittenRandAmount += amountWritten;
        if (amountWritten > randomCount)
        {
            break;
        }

        randomCount -= amountWritten;
    }
}

/**
 * getRandomOstream(int, ostream) will break the input amount down to legal random API
 * chunks and push the random data in the set format to the output stream passed in.
 */
void HttpRandomGetterImpl::getRandomOstream(uint64_t randomAmount, std::ostream &outputRandomFile)
{
    lastWrittenRandAmount = 0;
    const char *fake_metadata = "No meta data";
    uint64_t randomCount = randomAmount;
    while (randomCount > 0)
    {
        // find number of KB to get, rounding up to the next KB (limited by max random length)
        uint64_t dataToGet = (std::min)(
            ((randomCount + static_cast<uint64_t>(meteringclientlib::B_TO_KB) - 1) / static_cast<uint64_t>(meteringclientlib::B_TO_KB)),
            static_cast<uint64_t>(HttpRandomGetter::RANDOM_URI_MAX_SIZE));
        // try to get dataToGet amount
        getRandom(dataToGet);
        // write returns actual dataToGet amount made available
        uint64_t amountWritten = writeToOstream(outputRandomFile);
        lastWrittenRandAmount += amountWritten;
        if (amountWritten > randomCount)
        {
            break;
        }

        randomCount -= amountWritten;
        // display progress of random download
        printProgress(randomAmount - randomCount, randomAmount);
        outputRandomFile.flush();
    }
    printProgress(randomAmount, randomAmount);

    if (lastWrittenRandAmount > 0)
    {
        outputRandomFile.flush();
    }
}

// This is the write callback to pass to the CurlWrapper to use.
std::size_t HttpRandomGetterImpl::writeCallback(char *contents, std::size_t size, std::size_t nmemb, std::string *userp)
{
    (userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

//This function prints the random outputs download progress to the terminal for the user to see
void HttpRandomGetterImpl::printProgress(uint64_t randomRecieved, uint64_t totalRandom)
{
    int barWidth = 20;
    float progress = static_cast<long double>(randomRecieved) / static_cast<long double>(totalRandom);
    int pos = progress * barWidth;
    std::cout.flush();
    //print actual download bar
    std::cout << "[";
    for (int i = 0; i < barWidth; ++i)
    {
        if (i < pos)
        {
            std::cout << "=";
        }
        else if (i == pos)
        {
            std::cout << ">";
        }
        else
        {
            std::cout << " ";
        }
    }
    std::cout << "] " << int(progress * 100.0) << " %\r";
    if (progress == 1.0)
    {
        std::cout << std::endl;
    }
}

/**
 * setRandomParams sets up the url to connect to the load balancer correctly.
 * ascii and binary outputs requires a decimal return from the load balancer
 * since those are easier to parse then a string. This should only be called
 * when the clientConfig has new values to use.
 */
void HttpRandomGetterImpl::setRandomParams()
{
    rpsUrlStart = apiEndpoint + HttpRandomGetter::RANDOM_URI;

    randomUri = rpsUrlStart;
}
} // namespace detail

// These constants were derivied from a sample curl call.
const char *HttpRandomGetter::RANDOM_URI = "/api/v1/quantum-entropy";
const int HttpRandomGetter::RANDOM_URI_MAX_SIZE;

HttpRandomGetter::HttpRandomGetter(const char *inputApi, const char *inputToken, const char *inputLogLocation)
    : hiddenImpPtr(inputApi, inputToken, inputLogLocation, "")
{
}

HttpRandomGetter::HttpRandomGetter(const char *inputApi, const char *inputToken, const char *inputLogLocation, const char *inputCaPath)
    : hiddenImpPtr(inputApi, inputToken, inputLogLocation, inputCaPath)
{
}

HttpRandomGetter::~HttpRandomGetter() = default;

HttpRandomGetter::HttpRandomGetter(HttpRandomGetter &&rhs) noexcept = default;
HttpRandomGetter &HttpRandomGetter::operator=(HttpRandomGetter &&rhs) noexcept = default;

void HttpRandomGetter::getRandomFile(uint64_t randomAmount, const char *randomOutputLocation)
{
    hiddenImpPtr->getRandomFile(randomAmount, randomOutputLocation);
}

void HttpRandomGetter::getRandomBuffer(uint64_t randomAmount, uint8_t *inputBuffer)
{
    hiddenImpPtr->getRandomBuffer(randomAmount, inputBuffer);
}

uint64_t HttpRandomGetter::getLastWrittenRandAmount()
{
    return hiddenImpPtr->getLastWrittenRandAmount();
}
} // namespace meteringclientlib
