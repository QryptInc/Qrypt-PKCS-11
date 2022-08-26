/**
 * This class contains the global state that must be
 * maintained by Qryptoki.
 * 
 * Singleton design pattern: https://stackoverflow.com/a/1008289
 */

#ifndef _QRYPT_WRAPPER_GLOBALDATA_H
#define _QRYPT_WRAPPER_GLOBALDATA_H

#include "cryptoki.h"         // PKCS#11 types

#include "BaseHSM.h"          // BaseHSM
#include "RandomCollector.h"  // RandomCollector
#include "RandomBuffer.h"     // RandomBuffer

class GlobalData {
    public:
        static GlobalData& getInstance() {
            static GlobalData instance;
            return instance;
        }

        GlobalData(GlobalData const&)     = delete;
        void operator=(GlobalData const&) = delete;

        CK_RV initialize(CK_C_INITIALIZE_ARGS_PTR pInitArgs);
        CK_RV finalize();

        bool isCryptokiInitialized();
        void *getBaseFunction(std::string fn_name);

        CK_RV lockRandomBufferMutex();
        CK_RV unlockRandomBufferMutex();

        CK_RV getRandom(CK_BYTE_PTR data, CK_ULONG len);
    private:
        GlobalData();
        ~GlobalData(){};

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