/**
 * This is a base class for both MeteringClientWrapper and
 * MockRandomCollector (in gtests/). Mocking out the
 * collection of entropy allows us to test the RandomBuffer
 * without calling EaaS every time.
 */

#ifndef _QRYPT_RANDOM_COLLECTOR_H
#define _QRYPT_RANDOM_COLLECTOR_H

#include "cryptoki.h"   // CK_RV

const uint64_t KB = 1024;

class RandomCollector {
    public:
        virtual ~RandomCollector() {};

        virtual CK_RV collectRandom(uint8_t *dest, size_t goal) = 0;
};

#endif /* !_QRYPT_RANDOM_COLLECTOR_H */