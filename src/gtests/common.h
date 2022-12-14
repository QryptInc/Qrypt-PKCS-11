#ifndef _QRYPT_WRAPPER_GTEST_COMMON_H
#define _QRYPT_WRAPPER_GTEST_COMMON_H

#include <memory>

#include "cryptoki.h"

static const char *GTEST_TOKEN_LABEL = "gtesttoken";
static const size_t GTEST_TOKEN_LABEL_LEN = 10;
static const char *GTEST_TOKEN_SO_PIN = "gtestso";
static const size_t GTEST_TOKEN_SO_PIN_LEN = 7;

static const char *EAAS_TOKEN_ENV_VAR = "QRYPT_EAAS_TOKEN";
static const char *BASE_HSM_ENV_VAR = "QRYPT_BASE_HSM_PATH";
static const char *CA_CERT_ENV_VAR = "QRYPT_CA_CERT_PATH";

static const char *EMPTY_TOKEN = "";
static const char *BOGUS_TOKEN = "bogustoken";
static const char *BLOCKED_TOKEN = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6IjJjMTAyNjc1MjRlNjQ4Y2U4YTIxMTI5YjdjNTk1MDg4In0.eyJleHAiOjE2OTQyNjg3MTQsIm5iZiI6MTY2MjczMjcxNCwiaXNzIjoiQVVUSCIsImlhdCI6MTY2MjczMjcxNCwiZ3JwcyI6WyJQVUIiXSwiYXVkIjpbIlJQUyJdLCJybHMiOlsiUk5EVVNSIl0sImNpZCI6InZxaE1Fb3VHeWx1d0g3Y1pPeTY1WiIsImR2YyI6IjU0NTc2YWQxYTA3ZTRiOTE4ODQ5YWE1OTk5OWQyNGIxIiwianRpIjoiMDBiOTU3ODJhNjAzNDAxZDk3NGY2MzIzNTc5MDk1NmMiLCJ0eXAiOjN9.KgiVmD_0716FODFF7Vh7LseKybHjYp5W9n1tV431m3JE7oxj55EoZpmpJatB3A58G2EgULyyR95ixFSaG9ddOA";
static const char *OUT_OF_ENTROPY_TOKEN = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6IjJjMTAyNjc1MjRlNjQ4Y2U4YTIxMTI5YjdjNTk1MDg4In0.eyJleHAiOjE2OTQyNzA4NTEsIm5iZiI6MTY2MjczNDg1MSwiaXNzIjoiQVVUSCIsImlhdCI6MTY2MjczNDg1MSwiZ3JwcyI6WyJQVUIiXSwiYXVkIjpbIlJQUyJdLCJybHMiOlsiUk5EVVNSIl0sImNpZCI6IjIwcE5yY3RNaFdlNFZOanlYejYyUyIsImR2YyI6ImUzYWFkMjIzMzE2ZDQ1ZTM4ZDY1OWYzNzc1MWJmMTE4IiwianRpIjoiZTcyYjdkMDNhYTcxNGI2OWExNTZkZDQ1NWUxNDk0YTciLCJ0eXAiOjN9.im_A0ycXZVy9bMbNHW2MqZc8g016Kd2s7g9eVrmYo-aMnlWbG6Ju_gI8BnLuYhqxyn0M75LAGwRtZGx20OzLWQ";
static const char *EXPIRED_TOKEN = "eyJhbGciOiJFUzI1NiIsInR5cCI6IkpXVCIsImtpZCI6IjJjMTAyNjc1MjRlNjQ4Y2U4YTIxMTI5YjdjNTk1MDg4In0.eyJleHAiOjE2NjI4MjA0NzksIm5iZiI6MTY2MjczNDA3OSwiaXNzIjoiQVVUSCIsImlhdCI6MTY2MjczNDA3OSwiZ3JwcyI6WyJQVUIiXSwiYXVkIjpbIlJQUyJdLCJybHMiOlsiUk5EVVNSIl0sImNpZCI6InZxaE1Fb3VHeWx1d0g3Y1pPeTY1WiIsImR2YyI6IjU0NTc2YWQxYTA3ZTRiOTE4ODQ5YWE1OTk5OWQyNGIxIiwianRpIjoiMDQwZjIyYTdlNTg5NGExNmEyNTQ0ZmM3OWQ3MmMzOTgiLCJ0eXAiOjN9.8Vzs5MFNeSpNsAmaIag38yv_n05VT0E6VU9GI8_brJ-Sr84edJTDLBX9tZ52Wp5NnU315OeCvM6a-A7uV71Bfw";

static const char *EMPTY_PATH = "";
static const char *BOGUS_PATH = "boguspath";

std::unique_ptr<char[]> setEnvVar(const char *var_name, const char *new_value);
void revertEnvVar(const char *var_name, std::unique_ptr<char[]> &stashed_value);

CK_RV initializeSingleThreaded();
CK_RV initializeMultiThreaded();

bool isGTestSlot(CK_SLOT_ID slotID);
bool isEmptySlot(CK_SLOT_ID slotID);

CK_RV newGTestSlot(CK_SLOT_ID &slotID);
CK_RV getGTestSlot(CK_SLOT_ID &slotID);

CK_RV newSession(CK_SLOT_ID slotID, CK_SESSION_HANDLE &session);

CK_RV finalize();

#endif // !_QRYPT_WRAPPER_GTEST_COMMON_H