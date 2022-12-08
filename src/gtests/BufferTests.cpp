#include <algorithm>    /* std::fill_n */
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <optional>     /* std::optional */

#include "qryptoki_pkcs11_vendor_defs.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "MockRandomCollector.h"
#include "RandomBuffer.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::SetArrayArgument;
using ::testing::Return;
using ::testing::InSequence;

TEST(BufferTests, All255) {
    uint8_t dest[20] = {0};

    uint8_t very_random[1024] = {0};
    std::fill_n(very_random, 1024, (uint8_t)255);

    std::shared_ptr<MockRandomCollector> randomCollector = std::make_shared<MockRandomCollector>();

    EXPECT_CALL(*randomCollector, collectRandom(_, 1024))
        .WillOnce(DoAll(SetArrayArgument<0>(very_random, &very_random[1024]),
                        Return(CKR_OK)));

    std::optional<RandomBuffer> randomBuffer;
    ASSERT_NO_THROW(randomBuffer.emplace(randomCollector));
    ASSERT_TRUE(randomBuffer.has_value());

    CK_RV rv = randomBuffer->getRandom(dest, 20);

    EXPECT_EQ(rv, CKR_OK);

    for(size_t i = 0; i < 20; i++) {
        EXPECT_EQ(dest[i], (uint8_t)255);
    }
}

TEST(BufferTests, CollectReturnsNotOk) {
    uint8_t dest[20] = {0};

    std::shared_ptr<MockRandomCollector> randomCollector = std::make_shared<MockRandomCollector>();

    EXPECT_CALL(*randomCollector, collectRandom(_, 1024))
        .WillOnce(Return(CKR_QRYPT_TOKEN_INVALID));

    std::optional<RandomBuffer> randomBuffer;
    ASSERT_NO_THROW(randomBuffer.emplace(randomCollector));
    ASSERT_TRUE(randomBuffer.has_value());

    CK_RV rv = randomBuffer->getRandom(dest, 20);

    EXPECT_EQ(rv, CKR_QRYPT_TOKEN_INVALID);

    for(size_t i = 0; i < 20; i++) {
        EXPECT_EQ(dest[i], 0);
    }
}

TEST(BufferTests, AllNonzero) {
    srand(time(NULL));

    uint8_t dest[2000] = {0};

    uint8_t very_random[2048] = {0};
    for(size_t i = 0; i < 2048; i++)
        very_random[i] = (rand() % 255) + 1;

    std::shared_ptr<MockRandomCollector> randomCollector = std::make_shared<MockRandomCollector>();

    EXPECT_CALL(*randomCollector, collectRandom(_, 1024))
        .WillOnce(DoAll(SetArrayArgument<0>(very_random, &very_random[1024]),
                        Return(CKR_OK)))
        .WillOnce(DoAll(SetArrayArgument<0>(&very_random[1024], &very_random[2048]),
                        Return(CKR_OK)));

    std::optional<RandomBuffer> randomBuffer;
    ASSERT_NO_THROW(randomBuffer.emplace(randomCollector));
    ASSERT_TRUE(randomBuffer.has_value());

    for(size_t iteration = 0; iteration < 400; iteration++) {
        CK_RV rv = randomBuffer->getRandom(&dest[5 * iteration], 5);
        EXPECT_EQ(rv, CKR_OK);
    }

    for(size_t i = 0; i < 2000; i++) {
        EXPECT_NE(dest[i], 0);
    }
}

TEST(BufferTests, MoreThan1024) {
    srand(time(NULL));

    uint8_t dest[10000] = {0};
    
    uint8_t very_random[10240] = {0};

    for(size_t i = 0; i < 10240; i++)
        very_random[i] = rand() % 256;
    
    std::shared_ptr<MockRandomCollector> randomCollector = std::make_shared<MockRandomCollector>();

    {
        InSequence seq;

        EXPECT_CALL(*randomCollector, collectRandom(_, 4096))
            .WillOnce(DoAll(SetArrayArgument<0>(very_random, &very_random[4096]),
                            Return(CKR_OK)));
        EXPECT_CALL(*randomCollector, collectRandom(_, 2048))
            .WillOnce(DoAll(SetArrayArgument<0>(&very_random[4096], &very_random[6144]),
                            Return(CKR_OK)));
        EXPECT_CALL(*randomCollector, collectRandom(_, 1024))
            .WillOnce(DoAll(SetArrayArgument<0>(&very_random[6144], &very_random[7168]),
                            Return(CKR_OK)));
        EXPECT_CALL(*randomCollector, collectRandom(_, 3072))
            .WillOnce(DoAll(SetArrayArgument<0>(&very_random[7168], &very_random[10240]),
                            Return(CKR_OK)));
    }

    std::optional<RandomBuffer> randomBuffer;
    ASSERT_NO_THROW(randomBuffer.emplace(randomCollector));
    ASSERT_TRUE(randomBuffer.has_value());

    CK_RV rv = randomBuffer->getRandom(dest, 4000);
    EXPECT_EQ(rv, CKR_OK);

    rv = randomBuffer->getRandom(&dest[4000], 2000);
    EXPECT_EQ(rv, CKR_OK);

    rv = randomBuffer->getRandom(&dest[6000], 1000);
    EXPECT_EQ(rv, CKR_OK);

    rv = randomBuffer->getRandom(&dest[7000], 3000);
    EXPECT_EQ(rv, CKR_OK);
}

/**
 * BEWARE! This test makes some assumptions about the buffer implementation.
 *  (1) Reads/writes aer to consecutive spaces in the buffer.
 *  (2) If requests sizes are all multiples of 8 bytes, buffer reads/writes will be 8-aligned.
 * 
 * These assumptions aside, this test is super valuable because it confirms that random is not
 * reused. 
 */
TEST(BufferTests, NoReuse) {
    srand(time(NULL));

    size_t total_random_in_64_bits;
    size_t sum_request_sizes_in_64_bits = 0;

    const size_t NUM_REQUESTS = 1000;
    const size_t MAX_REQUEST_SIZE_IN_BYTES = 2048;
    size_t request_sizes_in_bytes[NUM_REQUESTS];

    for(size_t i = 0; i < NUM_REQUESTS; i++) {
        size_t request_size_in_64_bits = (rand() % MAX_REQUEST_SIZE_IN_BYTES) / 8;
        sum_request_sizes_in_64_bits += request_size_in_64_bits;

        request_sizes_in_bytes[i] = 8 * request_size_in_64_bits;
    }

    total_random_in_64_bits = (sum_request_sizes_in_64_bits + (KB / 8) - 1) / (KB / 8) * (KB / 8);

    std::unique_ptr<uint64_t[]> random_stream_unique_ptr = std::make_unique<uint64_t[]>(total_random_in_64_bits);
    uint64_t *random_stream_64_bits = random_stream_unique_ptr.get();
    uint8_t *random_stream_bytes = (uint8_t *)random_stream_64_bits;

    for(size_t i = 0; i < total_random_in_64_bits; i++) {
        random_stream_64_bits[i] = i + 1; // very random indeed!
    }

    std::shared_ptr<MockRandomCollector> randomCollector = std::make_shared<MockRandomCollector>();

    {
        InSequence seq;

        size_t random_stream_bytes_idx = 0;
        size_t bytes_in_buffer = 0;

        for(size_t i = 0; i < NUM_REQUESTS; i++) {
            size_t request_bytes = request_sizes_in_bytes[i];

            if(request_bytes <= bytes_in_buffer) {
                // No call to RandomCollector.collectRandom
                bytes_in_buffer -= request_bytes;
            } else {
                size_t bytes_from_buffer = bytes_in_buffer;
                size_t bytes_from_collector = request_bytes - bytes_from_buffer;
                size_t round_up = ((bytes_from_collector + KB - 1) / KB) * KB;

                uint8_t *random_start = &random_stream_bytes[random_stream_bytes_idx];
                uint8_t *random_end = &random_start[round_up];

                EXPECT_CALL(*randomCollector, collectRandom(_, round_up))
                    .WillOnce(DoAll(SetArrayArgument<0>(random_start, random_end),
                                    Return(CKR_OK)));
                
                random_stream_bytes_idx += round_up;
                bytes_in_buffer = round_up - bytes_from_collector;
            }

            EXPECT_EQ(random_stream_bytes_idx % 1024, 0);
            EXPECT_EQ(bytes_in_buffer % 8, 0);
        }
    }

    std::optional<RandomBuffer> randomBuffer;
    ASSERT_NO_THROW(randomBuffer.emplace(randomCollector));
    ASSERT_TRUE(randomBuffer.has_value());

    std::unique_ptr<uint64_t[]> output_stream_unique_ptr = std::make_unique<uint64_t[]>(sum_request_sizes_in_64_bits);
    uint64_t *output_stream_64_bits = output_stream_unique_ptr.get();
    uint8_t *output_stream_bytes = (uint8_t *)output_stream_64_bits;

    size_t output_stream_bytes_idx = 0;

    for(size_t i = 0; i < NUM_REQUESTS; i++) {
        uint8_t *output_begin = &output_stream_bytes[output_stream_bytes_idx];
        size_t request_size_in_bytes = request_sizes_in_bytes[i];

        CK_RV rv = randomBuffer->getRandom(output_begin, request_size_in_bytes);
        EXPECT_EQ(rv, CKR_OK);

        output_stream_bytes_idx += request_size_in_bytes;
    }

    // Possible values in output_stream go from 1 to total_random_in_64_bits
    // We expect 0 to never appear and anything from 1 to total_random_in_64_bits
    // to appear at most once.

    std::unique_ptr<bool[]> seen = std::make_unique<bool[]>(total_random_in_64_bits + 1);
    for(size_t i = 0; i < total_random_in_64_bits + 1; i++)
        seen[i] = false;

    for(size_t i = 0; i < sum_request_sizes_in_64_bits; i++) {
        uint64_t val = output_stream_64_bits[i];

        EXPECT_FALSE(seen[val]);
        seen[val] = true;
    }

    EXPECT_FALSE(seen[0]);
}
