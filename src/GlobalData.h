#ifndef _QRYPT_WRAPPER_GLOBALDATA_H
#define _QRYPT_WRAPPER_GLOBALDATA_H

#include "cryptoki.h"
#include "BaseHSM.h"
#include "RandomBuffer.h"
#include "RandomCollector.h"

class GlobalData {
    public:
        GlobalData();
        ~GlobalData(){};

        CK_RV initialize(CK_C_INITIALIZE_ARGS_PTR pInitArgs);
        CK_RV finalize();

        bool isCryptokiInitialized();
        void *getBaseFunction(std::string fn_name);

        CK_RV lockRandomBufferMutex();
        CK_RV unlockRandomBufferMutex();

        CK_RV getRandom(CK_BYTE_PTR data, CK_ULONG len);
    private:
        BaseHSM *baseHSM;

        // Mutex stuff
        bool isMultithreaded;

        CK_CREATEMUTEX customCreateMutex;
        CK_DESTROYMUTEX customDestroyMutex;
        CK_LOCKMUTEX customLockMutex;
        CK_UNLOCKMUTEX customUnlockMutex;

        CK_RV getThreadSettings(CK_C_INITIALIZE_ARGS_PTR pInitArgs);

        CK_RV createMutexIfNecessary(CK_VOID_PTR_PTR ppMutex);
        CK_RV destroyMutexIfNecessary(CK_VOID_PTR pMutex);
        CK_RV lockMutexIfNecessary(CK_VOID_PTR pMutex);
        CK_RV unlockMutexIfNecessary(CK_VOID_PTR pMutex);

        // Random buffer stuff
        CK_VOID_PTR randomBufferMutex;

        RandomCollector *randomCollector;
        RandomBuffer *randomBuffer;
        CK_RV setupRandomBuffer();
};

#endif /* !_QRYPT_WRAPPER_GLOBALDATA_H */