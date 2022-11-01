#include <iostream>
#include <thread>
#include <set>

#include "qryptoki_pkcs11_vendor_defs.h"

#include "common.h"
#include "run_tests.h"

namespace generaterandomtests {

bool allZeroes(CK_BYTE_PTR data, CK_ULONG len) {
    for(CK_ULONG i = 0; i < len; i++) {
        if(data[i] != (CK_BYTE)0)
            return false;
    }
    
    return true;
}

bool notInitialized(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    CK_SESSION_HANDLE session = CK_INVALID_HANDLE;

    const size_t len = 40;
    CK_BYTE data[len] = {0};

    rv = fn_list->C_GenerateRandom(session, data, len);

    return (rv == CKR_CRYPTOKI_NOT_INITIALIZED) && allZeroes(data, len);
}

bool sessionNotStarted(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session = CK_INVALID_HANDLE;

    const size_t len = 40;
    CK_BYTE data[len] = {0};

    rv = fn_list->C_GenerateRandom(session, data, len);

    return (rv == CKR_SESSION_HANDLE_INVALID) && allZeroes(data, len);
}

bool sessionClosed(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;
    
    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;

    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    rv = fn_list->C_CloseSession(session);
    if(rv != CKR_OK) return false;

    const size_t len = 40;
    CK_BYTE data[len] = {0};

    rv = fn_list->C_GenerateRandom(session, data, len);

    return (rv == CKR_SESSION_HANDLE_INVALID) && allZeroes(data, len);
}

bool emptyToken(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;
    
    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;
    
    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    
    std::unique_ptr<char[]> stashed_token = setEnvVar(EAAS_TOKEN_ENV_VAR, EMPTY_TOKEN);

    rv = fn_list->C_GenerateRandom(session, data, len);

    revertEnvVar(EAAS_TOKEN_ENV_VAR, stashed_token);

    return (rv == CKR_QRYPT_TOKEN_EMPTY) && allZeroes(data, len);
}

bool bogusToken(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;
    
    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;
    
    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    
    std::unique_ptr<char[]> stashed_token = setEnvVar(EAAS_TOKEN_ENV_VAR, BOGUS_TOKEN);

    rv = fn_list->C_GenerateRandom(session, data, len);

    revertEnvVar(EAAS_TOKEN_ENV_VAR, stashed_token);

    return (rv == CKR_QRYPT_TOKEN_INVALID) && allZeroes(data, len);
}

bool expiredToken(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;
    
    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;
    
    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    const size_t len = 2000;
    CK_BYTE data[len] = {0};

    std::unique_ptr<char[]> stashed_token = setEnvVar(EAAS_TOKEN_ENV_VAR, EXPIRED_TOKEN);

    rv = fn_list->C_GenerateRandom(session, data, len);

    revertEnvVar(EAAS_TOKEN_ENV_VAR, stashed_token);

    return (rv == CKR_QRYPT_TOKEN_INVALID) && allZeroes(data, len);
}

bool bogusCACert(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;
    
    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;
    
    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    const size_t len = 40;
    CK_BYTE data[len] = {0};
    
    std::unique_ptr<char[]> stashed_ca_cert = setEnvVar(CA_CERT_ENV_VAR, BOGUS_PATH);

    rv = fn_list->C_GenerateRandom(session, data, len);

    revertEnvVar(CA_CERT_ENV_VAR, stashed_ca_cert);

    return (rv == CKR_QRYPT_CA_CERT_FAILURE && allZeroes(data, len))
        || (rv == CKR_OK && !allZeroes(data, len));
}

bool badArgs(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;
    
    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;
    
    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    const size_t len = 2000;
    
    rv = fn_list->C_GenerateRandom(session, NULL_PTR, len);

    return rv == CKR_ARGUMENTS_BAD;
}

bool validRequestBasic(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;
    
    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;
    
    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    const size_t len = 2000;
    CK_BYTE data[len] = {0};
    
    rv = fn_list->C_GenerateRandom(session, data, len);

    return (rv == CKR_OK) && !allZeroes(data, len);
}

bool validRequestLengthZero(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeSingleThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;
    
    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    CK_SESSION_HANDLE session;
    
    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return false;

    const size_t len = 0;
    CK_BYTE data[len] = {};
    
    rv = fn_list->C_GenerateRandom(session, data, len);

    return rv == CKR_OK;
}

void getRandom(CK_FUNCTION_LIST_PTR fn_list, CK_SLOT_ID slotID, CK_BYTE_PTR data, CK_ULONG len, CK_RV &rv) {
    CK_SESSION_HANDLE session;

    rv = newSession(fn_list, slotID, session);
    if(rv != CKR_OK) return;

    rv = fn_list->C_GenerateRandom(session, data, len);
}

bool validRequestManyThreads(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    rv = initializeMultiThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;

    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;
    
    const size_t NUM_THREADS = 10;
    std::thread threads[NUM_THREADS];
    CK_RV rvs[NUM_THREADS];

    const size_t len = 300;
    CK_BYTE data[NUM_THREADS][len];

    for(size_t i = 0; i < NUM_THREADS; i++) {
        for(size_t j = 0; j < len; j++) {
            data[i][j] = (CK_BYTE)0;
        }
    }

    for(size_t i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(getRandom, fn_list, slotID, data[i], len, std::ref(rvs[i]));
    }

    for(size_t i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    for(size_t i = 0; i < NUM_THREADS; i++) {
        if(rvs[i] != CKR_OK) return false;
        if(allZeroes(data[i], len)) return false;
    }

    return true;
}

bool noReuseManyThreads(CK_FUNCTION_LIST_PTR fn_list) {
    CK_RV rv;
    finalize(fn_list);

    const size_t NUM_THREADS = 100;
    const size_t MAX_REQUEST_SIZE_IN_BYTES = 256;
    std::thread threads[NUM_THREADS];
    CK_RV rvs[NUM_THREADS];

    const size_t BYTES_PER_64_BITS = 8;

    rv = initializeMultiThreaded(fn_list);
    if(rv != CKR_OK) return false;

    CK_SLOT_ID slotID;

    rv = getIntegrationTestSlot(fn_list, slotID);
    if(rv != CKR_OK) return false;

    size_t request_sizes_in_bytes[NUM_THREADS];
    CK_BYTE data[NUM_THREADS][MAX_REQUEST_SIZE_IN_BYTES];

    for(size_t i = 0; i < NUM_THREADS; i++) {
        size_t size_in_64_bits = (rand() % MAX_REQUEST_SIZE_IN_BYTES) / BYTES_PER_64_BITS;
        request_sizes_in_bytes[i] = BYTES_PER_64_BITS * size_in_64_bits;

        for(size_t j = 0; j < MAX_REQUEST_SIZE_IN_BYTES; j++) {
            data[i][j] = (CK_BYTE)0;
        }
    }

    for(size_t i = 0; i < NUM_THREADS; i++) {
        threads[i] = std::thread(getRandom, fn_list, slotID, data[i], request_sizes_in_bytes[i], std::ref(rvs[i]));
    }

    for(size_t i = 0; i < NUM_THREADS; i++) {
        threads[i].join();
    }

    for(size_t i = 0; i < NUM_THREADS; i++) {
        if(rvs[i] != CKR_OK) return false;
        if(request_sizes_in_bytes[i] != 0 && allZeroes(data[i], request_sizes_in_bytes[i]))
            return false;
    }

    std::set<uint64_t> seen;

    for(size_t i = 0; i < NUM_THREADS; i++) {
        uint64_t *data_in_64_bits = (uint64_t *)data[i];

        for(size_t j =  0; j < request_sizes_in_bytes[i] / BYTES_PER_64_BITS; j++) {
            if(seen.count(data_in_64_bits[j]) != 0) return false;
            seen.insert(data_in_64_bits[j]);
        }
    }

    return true;
}

} // generaterandomtests

void runGenerateRandomTests(CK_FUNCTION_LIST_PTR fn_list,
                            size_t &tests_run_acc,
                            size_t &tests_passed_acc) {
    size_t failed = 0;
    size_t passed = 0;

    std::cout << "GENERATE RANDOM" << std::endl;

    if(!generaterandomtests::notInitialized(fn_list)) {
        std::cout << "notInitialized test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::sessionNotStarted(fn_list)) {
        std::cout << "sessionNotStarted test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::sessionClosed(fn_list)) {
        std::cout << "sessionClosed test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::emptyToken(fn_list)) {
        std::cout << "emptyToken test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::bogusToken(fn_list)) {
        std::cout << "bogusToken test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::expiredToken(fn_list)) {
        std::cout << "expiredToken test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::bogusCACert(fn_list)) {
        std::cout << "bogusCACert test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::badArgs(fn_list)) {
        std::cout << "badArgs test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::validRequestBasic(fn_list)) {
        std::cout << "validRequestBasic test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::validRequestLengthZero(fn_list)) {
        std::cout << "validRequestLengthZero test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::validRequestManyThreads(fn_list)) {
        std::cout << "validRequestManyThreads test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    if(!generaterandomtests::noReuseManyThreads(fn_list)) {
        std::cout << "noReuseManyThreads test failed." << std::endl;
        failed++;
    } else {
        passed++;
    }

    std::cout << "Tests run: " << (passed + failed) << std::endl;
    std::cout << "Tests passed: " << passed << std::endl << std::endl;

    tests_run_acc += (passed + failed);
    tests_passed_acc += passed;

    finalize(fn_list);
}
