/**
 * This class handles initializing and retrieving function pointers
 * from the base HSM.
 */

#ifndef _QRYPT_BASEHSM_H
#define _QRYPT_BASEHSM_H

#include <string>     // std::string

#include "cryptoki.h" // PKCS#11 types

class BaseHSM {
    public:
        BaseHSM();

        CK_RV initialize();
        bool isInitialized();
        
        void finalize();

        void *getFunction(std::string fn_name);
    private:
        void *handle;
        CK_FUNCTION_LIST_PTR baseFunctionList;
};

#endif /* !_QRYPT_BASEHSM_H */