#include <stdlib.h>
#include <dlfcn.h>

#include "log.h"
#include "osmutex.h"
#include "GlobalData.h"

GlobalData::GlobalData() {
    this->baseHSM = NULL;

    this->isMultithreaded = false;

    this->customCreateMutex = NULL;
    this->customDestroyMutex = NULL;
    this->customLockMutex = NULL;
    this->customUnlockMutex = NULL;

    this->randomBufferMutex = NULL;

    this->randomCollector = NULL;
    this->randomBuffer = NULL;
}

CK_RV GlobalData::getThreadSettings(CK_C_INITIALIZE_ARGS_PTR pInitArgs) {
    if(pInitArgs == NULL) {
        this->isMultithreaded = false;

        this->customCreateMutex = NULL;
        this->customDestroyMutex = NULL;
        this->customLockMutex = NULL;
        this->customUnlockMutex = NULL;
    } else {
        bool osLockingOk = pInitArgs->flags & CKF_OS_LOCKING_OK;

        CK_CREATEMUTEX create;
        CK_DESTROYMUTEX destroy;
        CK_LOCKMUTEX lock;
        CK_UNLOCKMUTEX unlock;

        create = pInitArgs->CreateMutex;
        destroy = pInitArgs->DestroyMutex;
        lock = pInitArgs->LockMutex;
        unlock = pInitArgs->UnlockMutex;

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
    this->baseHSM = new BaseHSM;
    CK_RV rv = this->baseHSM->initialize();
    if(rv != CKR_OK) return rv;

    rv = getThreadSettings(pInitArgs);
    if(rv != CKR_OK) return rv;

    // Create mutex for access to random buffer
    CK_VOID_PTR mutex;
    
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

    if(baseHSM != NULL) {
        delete baseHSM;
    }
    baseHSM = NULL;

    isMultithreaded = false;

    customCreateMutex = NULL;
    customDestroyMutex = NULL;
    customLockMutex = NULL;
    customUnlockMutex = NULL;

    randomBufferMutex = NULL;

    if(randomBuffer != NULL) {
        randomBuffer->wipe();
        delete randomBuffer;
    }

    randomBuffer = NULL;

    if(randomCollector != NULL) {
        delete randomCollector;
    }

    randomCollector = NULL;

    return CKR_OK;
}

bool GlobalData::isCryptokiInitialized() {
    return this->baseHSM != NULL;
}

void *GlobalData::getBaseFunction(std::string fn_name) {
    return this->baseHSM->getFunction(fn_name);
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

    this->randomCollector = new MeteringClientWrapper(token);

    try {
        this->randomBuffer = new RandomBuffer(this->randomCollector);
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
