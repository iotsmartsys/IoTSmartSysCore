#pragma once

#include <stdint.h>
#include "Contracts/Common/StateResult.h"

namespace iotsmartsys::core::settings
{

    enum class SettingsReadyLevel : uint8_t
    {
        /// @brief Nenhum estado de prontidão.
        None = 0,
        /// @brief Cache disponível (NVS).
        Available = 1,
        /// @brief API sincronizada (aplicado).
        Synced = 2,
        /// @brief Em processo de sincronização.
        Syncing = 3,
    };

    using SettingsGateCallback = void (*)(SettingsReadyLevel level, void *user_ctx);

    class ISettingsGate
    {
    public:
        virtual ~ISettingsGate() = default;

        virtual SettingsReadyLevel level() const = 0;

        // Sinais vindos do SettingsManager:
        /// <summary>
        /// Sinaliza que o cache está disponível.
        /// </summary>
        virtual void signalAvailable() = 0;

        /// <summary>
        /// Sinaliza que as configurações foram sincronizadas com a API.
        /// </summary>
        virtual void signalSynced() = 0;

        /// <summary>
        /// Sinaliza que as configurações estão em processo de sincronização com a API.
        /// </summary>
        virtual void signalSyncing() = 0;

        /// <summary>
        /// Sinaliza que houve um erro ao buscar ou aplicar as configurações.
        /// </summary>
        virtual void signalError(iotsmartsys::core::common::StateResult err) = 0;

        // Gate: executa cb quando atingir o nível desejado (ou imediatamente se já tiver atingido)
        virtual iotsmartsys::core::common::StateResult runWhenReady(
            SettingsReadyLevel want,
            SettingsGateCallback cb,
            void *user_ctx) = 0;
    };

} // namespace iotsmartsys::core::settings