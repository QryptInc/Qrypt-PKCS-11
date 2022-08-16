/**
 * Edited from file of the same name in QDARACLI -Sam
 */

#include <cstring>
#include <fstream>

#include "MeteringClientWrapper.h"
#include "HttpRandomGetter.h"
#include "log.h"

MeteringClientWrapper::MeteringClientWrapper(std::string token) {
    this->token = token;

    const char *cacert_path = std::getenv("QRYPT_CA_CERT_PATH");

	if(cacert_path == NULL || cacert_path[0] == '\0') {
        _HttpRandomGetter = std::make_shared<meteringclientlib::HttpRandomGetter>(prependHttps(EAAS_API_ENDPOINT).c_str(),
                                                                                  token.c_str(), "./");
	} else {
        _HttpRandomGetter = std::make_shared<meteringclientlib::HttpRandomGetter>(prependHttps(EAAS_API_ENDPOINT).c_str(),
                                                                                  token.c_str(), "./", cacert_path);
    }
}

MeteringClientWrapper::~MeteringClientWrapper() {}

std::string MeteringClientWrapper::prependHttps(std::string url) { return std::string("https://") + url; }

CK_RV MeteringClientWrapper::collectRandom(uint8_t *dest, size_t goal) const {
    if (goal == 0) return CKR_OK;
    
    size_t goalRoundUpToKB = ((goal + KB - 1) / KB) * KB;
    uint8_t* outputRoundUpKB = new uint8_t[goalRoundUpToKB];

    try {
        _HttpRandomGetter->getRandomBuffer(goalRoundUpToKB, outputRoundUpKB);
    } catch (std::exception &ex) {
        delete[] outputRoundUpKB;

        const char *msg = ex.what();

        if(strncmp("Reponse code: 401", msg, 17) == 0)
            return CKR_QRYPT_TOKEN_INVALID;
        else if(strncmp("Problem with the SSL CA cert", msg, 28) == 0)
            return CKR_QRYPT_CA_CERT_FAILURE;

        DEBUG_MSG("Unanticipated exception: %s", msg);
        return CKR_GENERAL_ERROR;
    }

    memcpy(dest, outputRoundUpKB, goal);
    delete[] outputRoundUpKB;

    return CKR_OK;
}
