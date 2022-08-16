#ifndef _QRYPT_RANDOM_COLLECTOR_H
#define _QRYPT_RANDOM_COLLECTOR_H

#include <string>

#include "cryptoki.h"

class RandomCollector {
    public:
        virtual ~RandomCollector() {};

        virtual CK_RV collectRandom(uint8_t *dest, size_t goal) const = 0;
};

#endif /* !_QRYPT_RANDOM_COLLECTOR_H */