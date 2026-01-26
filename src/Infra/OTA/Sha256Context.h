#pragma once

#include "mbedtls/sha256.h"
#include "mbedtls/version.h"

namespace iotsmartsys::ota
{

    struct Sha256Context
    {
        mbedtls_sha256_context ctx;
        void begin()
        {
            mbedtls_sha256_init(&ctx);
#if defined(MBEDTLS_VERSION_NUMBER) && (MBEDTLS_VERSION_NUMBER >= 0x03000000)
            mbedtls_sha256_starts(&ctx, 0);
#else
            mbedtls_sha256_starts_ret(&ctx, 0);
#endif
        }
        void update(const uint8_t *data, size_t len)
        {
#if defined(MBEDTLS_VERSION_NUMBER) && (MBEDTLS_VERSION_NUMBER >= 0x03000000)
            mbedtls_sha256_update(&ctx, data, len);
#else
            mbedtls_sha256_update_ret(&ctx, data, len);
#endif
        }
        void finish(uint8_t out[32])
        {
#if defined(MBEDTLS_VERSION_NUMBER) && (MBEDTLS_VERSION_NUMBER >= 0x03000000)
            mbedtls_sha256_finish(&ctx, out);
#else
            mbedtls_sha256_finish_ret(&ctx, out);
#endif
            mbedtls_sha256_free(&ctx);
        }
    };
} // namespace iotsmartsys::ota
