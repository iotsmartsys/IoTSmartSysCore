#pragma once

#if defined(ESP32)
#include "mbedtls/sha256.h"
#endif

namespace iotsmartsys::ota
{

#if defined(ESP32)
    struct Sha256Context
    {
        mbedtls_sha256_context ctx;
        void begin()
        {
            mbedtls_sha256_init(&ctx);
            mbedtls_sha256_starts_ret(&ctx, 0);
        }
        void update(const uint8_t *data, size_t len)
        {
            mbedtls_sha256_update_ret(&ctx, data, len);
        }
        void finish(uint8_t out[32])
        {
            mbedtls_sha256_finish_ret(&ctx, out);
            mbedtls_sha256_free(&ctx);
        }
    };
#elif defined(ESP8266)
    struct Sha256Context
    {
        BearSSL::HashSHA256 hasher;
        void begin()
        {
            hasher.begin();
        }
        void update(const uint8_t *data, size_t len)
        {
            hasher.add(data, len);
        }
        void finish(uint8_t out[32])
        {
            hasher.end();
            memcpy(out, hasher.hash(), hasher.len());
        }
    };
#endif
} // namespace iotsmartsys::ota