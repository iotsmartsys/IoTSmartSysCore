#pragma once

#include <cstdint>

namespace iotsmartsys::core::common
{
    enum class Error : std::uint8_t
    {
        Ok = 0,

        // Genéricos
        InvalidArg,
        InvalidState,
        NoMem,
        Timeout,
        Cancelled,
        NotFound,
        Overflow,
        ParseError,
        IoError,
        NotSupported,

        // Transporte/HTTP
        NetworkDown,
        DnsFail,
        ConnectFail,
        TlsFail,
        HttpError,

        // Persistência
        StorageCorrupt,
        StorageVersionMismatch,
        StorageWriteFail,
        StorageReadFail,

        // Fallback
        Unknown
    };

    inline bool ok(Error e) { return e == Error::Ok; }
} // namespace iotsmartsys::core::common