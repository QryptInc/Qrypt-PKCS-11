#ifndef _QRYPT_WRAPPER_GTEST_COMMON_H
#define _QRYPT_WRAPPER_GTEST_COMMON_H

#include "pkcs11.h"

static const char *GTEST_TOKEN_LABEL = "gtesttoken";
static const size_t GTEST_TOKEN_LABEL_LEN = 10;
static const char *GTEST_TOKEN_SO_PIN = "gtestso";
static const size_t GTEST_TOKEN_SO_PIN_LEN = 7;

static const char *EAAS_TOKEN_ENV_VAR = "QRYPT_EAAS_TOKEN";
static const char *BASE_HSM_ENV_VAR = "QRYPT_BASE_HSM_PATH";
static const char *CA_CERT_ENV_VAR = "QRYPT_CA_CERT_PATH";

static const char *EMPTY_TOKEN = "";
static const char *BOGUS_TOKEN = "bogustoken";
static const char *VALID_TOKEN = "PASTE A VALID QRYPT EAAS TOKEN HERE";
// ^ Change me!

static const char *EMPTY_PATH = "";
static const char *BOGUS_PATH = "boguspath";

CK_RV initializeSingleThreaded();
CK_RV initializeMultiThreaded();

bool isGTestSlot(CK_SLOT_ID slotID);
bool isEmptySlot(CK_SLOT_ID slotID);

CK_RV newGTestSlot(CK_SLOT_ID &slotID);
CK_RV getGTestSlot(CK_SLOT_ID &slotID);

CK_RV newSession(CK_SLOT_ID slotID, CK_SESSION_HANDLE &session);

CK_RV finalize();

#endif // !_QRYPT_WRAPPER_GTEST_COMMON_H