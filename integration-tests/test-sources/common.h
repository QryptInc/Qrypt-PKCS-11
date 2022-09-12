#ifndef _QRYPTOKI_INTEGRATION_TESTS_COMMON_H
#define _QRYPTOKI_INTEGRATION_TESTS_COMMON_H

#include <memory>

#include "cryptoki.h"

static const char *INTEGRATION_TESTS_TOKEN_LABEL = "integrationtoken                ";
static const char *INTEGRATION_TESTS_TOKEN_SO_PIN = "integrationso";

static const char *EAAS_TOKEN_ENV_VAR = "QRYPT_EAAS_TOKEN";
static const char *BASE_HSM_ENV_VAR = "QRYPT_BASE_HSM_PATH";
static const char *CA_CERT_ENV_VAR = "QRYPT_CA_CERT_PATH";

static const char *EMPTY_TOKEN = "";
static const char *BOGUS_TOKEN = "bogustoken";

static const char *EMPTY_PATH = "";
static const char *BOGUS_PATH = "boguspath";

std::unique_ptr<char[]> setEnvVar(const char *var_name, const char *new_value);
void revertEnvVar(const char *var_name, std::unique_ptr<char[]> &stashed_value);

CK_RV initializeSingleThreaded(CK_FUNCTION_LIST_PTR fn_list);
CK_RV initializeMultiThreaded(CK_FUNCTION_LIST_PTR fn_list);

bool isIntegrationTestSlot(CK_FUNCTION_LIST_PTR fn_list, CK_SLOT_ID slotID);
bool isEmptySlot(CK_FUNCTION_LIST_PTR fn_list, CK_SLOT_ID slotID);

CK_RV newIntegrationTestSlot(CK_FUNCTION_LIST_PTR fn_list, CK_SLOT_ID &slotID);
CK_RV getIntegrationTestSlot(CK_FUNCTION_LIST_PTR fn_list, CK_SLOT_ID &slotID);

CK_RV newSession(CK_FUNCTION_LIST_PTR fn_list,
                 CK_SLOT_ID slotID,
                 CK_SESSION_HANDLE &session);

CK_RV finalize(CK_FUNCTION_LIST_PTR fn_list);

#endif // !_QRYPTOKI_INTEGRATION_TESTS_COMMON_H