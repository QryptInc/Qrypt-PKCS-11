/**
 * This class manages Qryptoki's interaction with the MeteringClient.
 */

#ifndef _QRYPT_WRAPPER_METERINGCLIENT_H
#define _QRYPT_WRAPPER_METERINGCLIENT_H

#include <memory>     // std::shared_ptr
#include <string>     // std::string

#include "cryptoki.h"            // CK_RV
#include "HttpRandomGetter.h"    // meteringclientlib::HttpRandomGetter
#include "RandomCollector.h"     // RandomCollector

const std::string EAAS_API_ENDPOINT = "api-eus.qrypt.com";

// MeteringClientWrapper is a wrapper class to pull random from MeteringClient
class MeteringClientWrapper : public RandomCollector {
  private:
    // Object to pull random from MeteringClient
    std::shared_ptr<meteringclientlib::HttpRandomGetter> _HttpRandomGetter;

    // Prepends https:// to a URL
    std::string prependHttps(std::string url);

  public:
    // Token to access the endpoint
    std::string token;

    // Custom constructor
    MeteringClientWrapper(std::string token);

    // Custom destructor
    ~MeteringClientWrapper() override;

    // Collects QRandom from source
    CK_RV collectRandom(uint8_t *dest, size_t goal) const override;
};

#endif /* !_QRYPT_WRAPPER_METERINGCLIENT_H */