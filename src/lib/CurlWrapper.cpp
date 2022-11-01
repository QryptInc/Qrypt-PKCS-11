#include <cstring>     // strncmp

#include "qryptoki_pkcs11_vendor_defs.h" // CKR_QRYPT_*
#include "log.h"                         // logging macros
#include "base64.h"
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <curl/curl.h>

#include "CurlWrapper.h"

CurlWrapper::CurlWrapper(std::string token) {
    this->token = token;
    this->restJson.SetObject();

    const char *cacert_path_c_str = std::getenv("QRYPT_CA_CERT_PATH");
    this->cacert_path = cacert_path_c_str ? cacert_path_c_str : "";
}

CurlWrapper::~CurlWrapper() {}

CK_RV CurlWrapper::collectRandom(uint8_t *dest, size_t goal) {
    if (goal == 0) return CKR_OK;
    
    CurlResponse response = this->performCURL(goal);

    switch (response.code) {
            case -1: {
                // CURL error already logged in performCURL
                return CKR_GENERAL_ERROR;
            } break;
            case 200: {
                rapidjson::Document().Swap(restJson);
                restJson.Parse(response.buffer.c_str());
                if (restJson.HasParseError())
                {
                   DEBUG_MSG(::rapidjson::GetParseError_En(restJson.GetParseError()));
                   return CKR_GENERAL_ERROR;
                }
                else if (!restJson.IsObject())
                {
                    DEBUG_MSG("JSON document is not an object.");
                    return CKR_GENERAL_ERROR;
                }
                else if (!restJson.HasMember("random"))
                {
                    DEBUG_MSG("Missing random in REST response.");
                    return CKR_GENERAL_ERROR;
                }

                int bufferWritePos = 0;
                this->writeToBuffer(dest, bufferWritePos);
            } break;
            case 400: {
                 DEBUG_MSG("Bad Request. The request was malformed or otherwise unacceptable.");
                 return CKR_GENERAL_ERROR;
            } break;
            case 401: {
                 DEBUG_MSG("Unauthorized, invalid Credentials or problem with the SSL CA cert.");
                 return CKR_QRYPT_TOKEN_INVALID;
            } break;
            case 403: {
                 DEBUG_MSG("Unknown server failure %zu", response.code);
                 return CKR_QRYPT_TOKEN_OTHER_FAIL;
            } break;
            case 429: {
                 DEBUG_MSG("Forbidden, you have hit the rate limit.");
                 return CKR_GENERAL_ERROR;
            } break;
            default: {
                DEBUG_MSG("Unknown server response code %zu", response.code);
                return CKR_GENERAL_ERROR;
            } break;
    } // switch

    return CKR_OK;
}

std::size_t writeCallback(char *contents, std::size_t size, std::size_t nmemb, std::string *userp) {
    (userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

CurlResponse CurlWrapper::performCURL(size_t goal) {

    CurlResponse response = {};

    CURL *curlHandle = curl_easy_init();

    std::string fullURL = "https://api-eus.qrypt.com/api/v1/quantum-entropy?size=";
    size_t goalInKib = goal / KB;
    fullURL += std::to_string(goalInKib);
    
    std::string authorizationHeader = "Authorization: Bearer " + this->token;

    struct curl_slist *curlSlist = NULL;
    curlSlist = curl_slist_append(curlSlist, authorizationHeader.c_str());
    curlSlist = curl_slist_append(curlSlist, "Accept: application/json");
    curlSlist = curl_slist_append(curlSlist, "Content-Type: application/json");

    // LibCURL receive buffer size - smaller value will result in more calls to writeCallback
    // Set CURLOPT_VERBOSE to 1 for debugging.
    // curl_easy_setopt(curlHandle, CURLOPT_VERBOSE, 1);
    curl_easy_setopt(curlHandle, CURLOPT_BUFFERSIZE, 102400L);
    curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, curlSlist);
    curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, "curl/7.63.0");
    curl_easy_setopt(curlHandle, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(curlHandle, CURLOPT_HTTP_VERSION, (long)CURL_HTTP_VERSION_2TLS);
    curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curlHandle, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curlHandle, CURLOPT_NOSIGNAL, 1L);

    curl_easy_setopt(curlHandle, CURLOPT_URL, fullURL.c_str());
    curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, &writeCallback);
    curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &response.buffer);

    if(!this->cacert_path.empty()) {
        curl_easy_setopt(curlHandle, CURLOPT_CAINFO, this->cacert_path.c_str());
    }
#ifdef WIN32
    // Use native ca root on Windows
    else {
        curl_easy_setopt(curlHandle, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
    }
#endif

    // Perform CURL command
    CURLcode curlCode = curl_easy_perform(curlHandle); 
    if (curlCode) {
        DEBUG_MSG("Entropy failed from curl_easy_perform with error code %zu", curlCode);
        curl_slist_free_all(curlSlist);
        curl_easy_cleanup(curlHandle);
        response.code = -1; 
        response.buffer = ""; 
        return response; 
    }

    curlCode = curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &response.code);
    if (curlCode) {
        DEBUG_MSG("Entropy failed from curl_easy_getinfo with error code %zu", curlCode);
        curl_slist_free_all(curlSlist);
        curl_easy_cleanup(curlHandle);
    }

    DEBUG_MSG("Entropy completed with http response code %zu", curlCode);

    curl_slist_free_all(curlSlist);
    curl_easy_cleanup(curlHandle);
    return response;
}

/*
 * writeToBuffer should be called after a getRandom. This function is gated to only
 * deal with a binary output. This function pushes all the random in the rapid json
 * object into the buffer pointer. The passed in buffer position will be moved to
 * the end of all the data pushed into the buffer and returned.
 * Programmer beware, this function will blindly trust the passed in buffer pointer
 * and buffer write position.
 */
std::tuple<int, uint64_t> CurlWrapper::writeToBuffer(uint8_t *inputBuffer, int bufferWritePos)
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
