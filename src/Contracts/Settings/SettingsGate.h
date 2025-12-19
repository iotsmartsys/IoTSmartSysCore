#pragma once

#include <stdint.h>
#include "Contracts/Common/StateResult.h"

namespace iotsmartsys::core::settings
{

    enum class SettingsReadyLevel : uint8_t
    {
        None = 0,
        Available = 1, // cache OK (NVS)
        Synced = 2     // API OK (aplicado)
    };

    using SettingsGateCallback = void (*)(SettingsReadyLevel level, void *user_ctx);

    class ISettingsGate
    {
    public:
        virtual ~ISettingsGate() = default;

        virtual SettingsReadyLevel level() const = 0;

        // Sinais vindos do SettingsManager:
        virtual void signalAvailable() = 0; // cache carregou OK
        virtual void signalSynced() = 0;    // API OK (aplicado)
        virtual void signalError(iotsmartsys::core::common::StateResult err) = 0;

        // Gate: executa cb quando atingir o nível desejado (ou imediatamente se já tiver atingido)
        virtual iotsmartsys::core::common::StateResult runWhenReady(
            SettingsReadyLevel want,
            SettingsGateCallback cb,
            void *user_ctx) = 0;

        virtual void handle() = 0;
    };

} // namespace iotsmartsys::core::settings