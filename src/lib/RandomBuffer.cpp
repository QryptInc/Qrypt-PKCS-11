#include <cstring>         // memset
#include <stdexcept>       // std::runtime_error

#include "log.h"           // DEBUG_MSG

#include "RandomBuffer.h"

// Zero out buffer[lo, hi)
void zeroBuffer(uint8_t *buffer, size_t lo, size_t hi) {
    memset(&buffer[lo], 0, (hi - lo) * sizeof(uint8_t));
}

RandomBuffer::RandomBuffer(std::shared_ptr<RandomCollector> randomCollector) {
    this->randomCollector = randomCollector;

    // Allocate the buffer on a page boundary
    uint8_t *buffer_ptr = (uint8_t *)valloc(KB * sizeof(uint8_t));
    this->buffer = std::unique_ptr<uint8_t, BufferDeleter>(buffer_ptr);

    if(this->buffer.get() == NULL) {
        throw std::runtime_error("Could not valloc RandomBuffer");
    }

    // Lock the buffer so it won't go to disk
    if(mlock(buffer_ptr, KB * sizeof(uint8_t)) != 0) {
        throw std::runtime_error("Could not mlock RandomBuffer");
    }

    zeroBuffer(this->buffer.get(), 0, KB);
    this->buffer_len = 0;
}

RandomBuffer::~RandomBuffer() {
    zeroBuffer(this->buffer.get(), 0, KB);
    this->buffer_len = 0;
}

CK_RV RandomBuffer::getRandom(uint8_t *dest, size_t goal) {
    // First... take all you can/need from buffer

    size_t bytesFromBuffer = goal < this->buffer_len ? goal : this->buffer_len;

    size_t lo = this->buffer_len - bytesFromBuffer;
    memcpy(dest, &this->buffer.get()[lo], bytesFromBuffer);

    zeroBuffer(this->buffer.get(), lo, lo + bytesFromBuffer);
    this->buffer_len = lo;

    DEBUG_MSG("Put %zu bytes from buffer into output", bytesFromBuffer);

    if(goal == bytesFromBuffer) return CKR_OK;

    // Then... take from EaaS

    uint8_t *next_dest = &dest[bytesFromBuffer];
    size_t bytesFromEaaS = goal - bytesFromBuffer;
    size_t bytesFromEaasRoundUp = ((bytesFromEaaS + KB - 1) / KB) * KB;

    std::unique_ptr<uint8_t[]> outputEaaS = std::make_unique<uint8_t[]>(bytesFromEaasRoundUp);
    CK_RV rv = this->randomCollector->collectRandom(outputEaaS.get(), bytesFromEaasRoundUp);

    if(rv != CKR_OK) return rv;

    DEBUG_MSG("Pulled %zu bytes from EaaS", bytesFromEaasRoundUp);

    memcpy(next_dest, outputEaaS.get(), bytesFromEaaS);

    DEBUG_MSG("Put %zu bytes from EaaS into output", bytesFromEaaS);

    // ... and put leftovers in buffer

    size_t leftover = bytesFromEaasRoundUp - bytesFromEaaS;

    memcpy(this->buffer.get(), &outputEaaS.get()[bytesFromEaaS], leftover);
    this->buffer_len = leftover;

    DEBUG_MSG("Put %zu bytes from EaaS into buffer", leftover);

    return CKR_OK;
}

void RandomBuffer::wipe() {
    zeroBuffer(this->buffer.get(), 0, KB);
    this->buffer_len = 0;
}
