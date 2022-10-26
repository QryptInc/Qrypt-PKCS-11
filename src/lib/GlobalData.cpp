#include <stdlib.h>
#include <stdexcept>     // std::runtime_error

#include "qryptoki_pkcs11_vendor_defs.h" // CKR_QRYPT_*
#include "log.h"                         // logging macros
#include "osmutex.h"                     // mutex functions
#include "MeteringClientWrapper.h"       // MeteringClientWrapper
#include "CurlWrapper.h"                 // CurlWrapper

#include "GlobalData.h"

GlobalData::GlobalData() {
    this->isMultithreaded = false;

    this->customCreateMutex = NULL;
    this->customDestroyMutex = NULL;
    this->customLockMutex = NULL;
    this->customUnlockMutex = NULL;

    this->randomBufferMutex = NULL;

    this->randomCollector = std::shared_ptr<RandomCollector>(nullptr);
    this->randomBuffer = std::unique_ptr<RandomBuffer>(nullptr);
}

CK_RV GlobalData::setThreadSettings(CK_C_INITIALIZE_ARGS_PTR pInitArgs) {
    if(pInitArgs == NULL) {
        this->isMultithreaded = false;

        this->customCreateMutex = NULL;
        this->customDestroyMutex = NULL;
        this->customLockMutex = NULL;
        this->customUnlockMutex = NULL;
    } else {
        bool osLockingOk = pInitArgs->flags & CKF_OS_LOCKING_OK;

        CK_CREATEMUTEX create = pInitArgs->CreateMutex;
        CK_DESTROYMUTEX destroy = pInitArgs->DestroyMutex;
        CK_LOCKMUTEX lock = pInitArgs->LockMutex;
        CK_UNLOCKMUTEX unlock = pInitArgs->UnlockMutex;

        bool oneNonNull = create || destroy || lock || unlock;
        bool allNonNull = create && destroy && lock && unlock;

        if(oneNonNull && !allNonNull) return CKR_ARGUMENTS_BAD;

        this->isMultithreaded = osLockingOk || allNonNull;

        this->customCreateMutex = create;
        this->customDestroyMutex = destroy;
        this->customLockMutex = lock;
        this->customUnlockMutex = unlock;
    }

    return CKR_OK;
}

CK_RV GlobalData::initialize(CK_C_INITIALIZE_ARGS_PTR pInitArgs) {
    CK_RV rv = this->baseHSM.initialize();
    if(rv != CKR_OK) return rv;

    rv = setThreadSettings(pInitArgs);
    if(rv != CKR_OK) return rv;

    // Create mutex for access to random buffer
    CK_VOID_PTR mutex = NULL;
    
    rv = createMutexIfNecessary(&mutex);
    if(rv != CKR_OK) return rv;

    this->randomBufferMutex = mutex;

    return CKR_OK;
}

CK_RV GlobalData::finalize() {
    if(randomBufferMutex != NULL) {
        CK_RV rv = destroyMutexIfNecessary(randomBufferMutex);
        if(rv != CKR_OK) return rv;
    }

    baseHSM.finalize();

    isMultithreaded = false;

    customCreateMutex = NULL;
    customDestroyMutex = NULL;
    customLockMutex = NULL;
    customUnlockMutex = NULL;

    randomBufferMutex = NULL;

    randomBuffer.reset();
    randomCollector.reset();

    return CKR_OK;
}

bool GlobalData::isCryptokiInitialized() {
    return this->baseHSM.isInitialized();
}

void *GlobalData::getBaseFunction(std::string fn_name) {
    return this->baseHSM.getFunction(fn_name);
}

CK_RV GlobalData::createMutexIfNecessary(CK_VOID_PTR_PTR ppMutex) {
    if(ppMutex == NULL) return CKR_ARGUMENTS_BAD;

    if(!this->isMultithreaded) {
        *ppMutex = NULL;
        return CKR_OK;
    }

    if(this->customCreateMutex == NULL)
        return OSCreateMutex(ppMutex);
    else
        return (this->customCreateMutex)(ppMutex);
}

CK_RV GlobalData::destroyMutexIfNecessary(CK_VOID_PTR pMutex) {
    if(!this->isMultithreaded) return CKR_OK;

    if(this->customDestroyMutex == NULL)
        return OSDestroyMutex(pMutex);
    else
        return (this->customDestroyMutex)(pMutex);
}

CK_RV GlobalData::lockMutexIfNecessary(CK_VOID_PTR pMutex) {
    if(!this->isMultithreaded) return CKR_OK;

    if(this->customLockMutex == NULL)
        return OSLockMutex(pMutex);
    else
        return (this->customLockMutex)(pMutex);
}

CK_RV GlobalData::unlockMutexIfNecessary(CK_VOID_PTR pMutex) {
    if(!this->isMultithreaded) return CKR_OK;

    if(this->customUnlockMutex == NULL)
        return OSUnlockMutex(pMutex);
    else
        return (this->customUnlockMutex)(pMutex);
}

CK_RV GlobalData::lockRandomBufferMutex() {
    return lockMutexIfNecessary(this->randomBufferMutex);
}

CK_RV GlobalData::unlockRandomBufferMutex() {
    return unlockMutexIfNecessary(this->randomBufferMutex);
}

CK_RV GlobalData::setupRandomBuffer() {
    if(this->randomBuffer != NULL) return CKR_GENERAL_ERROR;

	const char *token_c_str = std::getenv("QRYPT_EAAS_TOKEN");

	if(token_c_str == NULL || token_c_str[0] == '\0')
		return CKR_QRYPT_TOKEN_EMPTY;

	std::string token(token_c_str);

    this->randomCollector = std::make_unique<CurlWrapper>(token);

    try {
        this->randomBuffer = std::make_unique<RandomBuffer>(this->randomCollector);
    } catch (std::runtime_error &ex) {
        // Failed to valloc or mlock buffer
        ERROR_MSG("%s", ex.what());
        return CKR_GENERAL_ERROR;
    }

    return CKR_OK;
}

CK_RV GlobalData::getRandom(CK_BYTE_PTR data, CK_ULONG len) {
    if(this->randomBuffer == NULL) {
        CK_RV rv = setupRandomBuffer();
        
        if(rv != CKR_OK) return rv;
    }

    return this->randomBuffer->getRandom(data, len);
}
