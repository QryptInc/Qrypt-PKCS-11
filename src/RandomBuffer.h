/**
 * This class stores extra random from EaaS to prevent waste.
 */

#ifndef _QRYPT_WRAPPER_RANDOMBUFFER_H
#define _QRYPT_WRAPPER_RANDOMBUFFER_H

#include <memory>

#include "cryptoki.h"          // CK_RV

#include "RandomCollector.h"   // RandomCollector

struct BufferDeleter {
    void operator() (uint8_t *buffer_ptr) const {
        free(buffer_ptr);
    }
};

class RandomBuffer {
    public:
        RandomBuffer(std::shared_ptr<RandomCollector> randomCollector);
        ~RandomBuffer();

        CK_RV getRandom(uint8_t *dest, size_t goal);
        void wipe();
    private:
        std::unique_ptr<uint8_t, BufferDeleter> buffer;
        size_t buffer_len;

        std::shared_ptr<RandomCollector> randomCollector;
};

#endif /* !_QRYPT_WRAPPER_RANDOMBUFFER_H */