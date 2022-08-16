#ifndef _QRYPT_BASEHSM_H
#define _QRYPT_BASEHSM_H

#include <string>

#include "cryptoki.h"

class BaseHSM {
    public:
        CK_RV initialize();
        void *getFunction(std::string fn_name);
    private:
        CK_FUNCTION_LIST_PTR baseFunctionList;
};

#endif /* !_QRYPT_BASEHSM_H */