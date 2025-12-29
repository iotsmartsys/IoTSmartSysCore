#pragma once
#include <functional>
#include "DeviceConfig.h"

namespace iotsmartsys::core::provisioning
{

    /// @brief Status de alto nível de um canal de provisionamento.
    enum class ProvisioningStatus
    {
        Idle,
        WaitingUserInput,
        ReceivingData,
        Validating,
        Applied,
        Failed
    };

    /// @brief Interface base para canais de provisionamento (BLE, Portal, Serial, etc.).
    class IProvisioningChannel
    {
    public:
        using ConfigCallback = std::function<void(const DeviceConfig &)>;
        using StatusCallback = std::function<void(ProvisioningStatus, const char *message)>;

        virtual ~IProvisioningChannel() = default;

        /// @brief Inicia o canal de provisionamento.
        virtual void begin() = 0;

        /// @brief Executa o ciclo de processamento do canal.
        virtual void loop() = 0;

        /// @brief Encerra o canal de provisionamento.
        virtual void stop() = 0;

        /// @brief Indica se o canal está ativo.
        virtual bool isActive() const = 0;

        /// @brief Define callback chamado quando uma nova configuração é recebida.
        virtual void onConfigReceived(ConfigCallback cb) = 0;

        /// @brief Define callback chamado quando o status do canal muda.
        virtual void onStatusChanged(StatusCallback cb) = 0;

        /// @brief Prioridade relativa do canal (valores maiores têm preferência).
        virtual uint8_t priority() const { return 0; }
    };

} // namespace iotsmartsys::core::provisioning