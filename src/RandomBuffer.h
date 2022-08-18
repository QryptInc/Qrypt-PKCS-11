#ifndef _QRYPT_WRAPPER_RANDOMBUFFER_H
#define _QRYPT_WRAPPER_RANDOMBUFFER_H

#include "cryptoki.h"          // CK_RV

#include "RandomCollector.h"   // RandomCollector

class RandomBuffer {
    public:
        RandomBuffer(RandomCollector *randomCollector);
        ~RandomBuffer();

        CK_RV getRandom(uint8_t *dest, size_t goal);
        void wipe();
    private:
        uint8_t *buffer;
        size_t buffer_len;

        RandomCollector *randomCollector;
};

#endif /* !_QRYPT_WRAPPER_RANDOMBUFFER_H */