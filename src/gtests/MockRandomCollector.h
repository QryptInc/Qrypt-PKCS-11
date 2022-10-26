#ifndef _QRYPT_MOCK_RANDOM_COLLECTOR_H
#define _QRYPT_MOCK_RANDOM_COLLECTOR_H

#include "gmock/gmock.h"
#include "cryptoki.h"
#include "RandomCollector.h"

class MockRandomCollector : public RandomCollector {
    public:
        MOCK_METHOD(CK_RV, collectRandom, (uint8_t *dest, size_t goal), (override));
};

#endif /* !_QRYPT_MOCK_RANDOM_COLLECTOR_H */