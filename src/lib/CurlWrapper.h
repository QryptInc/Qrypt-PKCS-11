/**
 * This class manages Qryptoki's interaction with the libcurl.
 */

#ifndef _CURL_WRAPPER_H
#define _CURL_WRAPPER_H

#include <memory>     // std::shared_ptr
#include <string>     // std::string

#include "cryptoki.h"           // CK_RV
#include "rapidjson/document.h" // json parsing
#include "RandomCollector.h"    // RandomCollector

struct CurlResponse {
    long code;
    std::string buffer;
};

// CurlWrapper is a wrapper class to pull random from EaaS
class CurlWrapper : public RandomCollector {
  public:
    // Custom constructor
    CurlWrapper(std::string token);

    // Custom destructor
    ~CurlWrapper() override;

    // Collects QRandom from source
    CK_RV collectRandom(uint8_t *dest, size_t goal) override;

  private:
    // Token to access the endpoint
    std::string token;

    std::string cacert_path;

    ::rapidjson::Document restJson;
    
    std::tuple<int, uint64_t> writeToBuffer(uint8_t *inputBuffer, int bufferWritePos);
    CurlResponse performCURL(size_t goal);
};

#endif /* !_CURL_WRAPPER_H */