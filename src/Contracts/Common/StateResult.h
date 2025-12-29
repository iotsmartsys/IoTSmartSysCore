#pragma once

#include <cstdint>

namespace iotsmartsys::core::common
{
    enum class StateResult : std::uint8_t
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

    inline bool ok(StateResult e) { return e == StateResult::Ok; }
} // namespace iotsmartsys::core::common