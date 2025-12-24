#pragma once
#include <vector>
#include <functional>
#include "DeviceConfig.h"
#include "IProvisioningChannel.h"
#include "Contracts/Logging/ILogger.h"

namespace iotsmartsys::core::provisioning
{

    /// @brief Orquestra o processo de provisionamento usando um ou mais canais.
    class ProvisioningManager
    {
    public:
        using ProvisioningCompletedCallback = std::function<void(const DeviceConfig &)>;
        using ChannelStatusCallback = std::function<void(const IProvisioningChannel &, ProvisioningStatus, const char *message)>;

        /// @brief Registra um canal de provisionamento disponível.
        void registerChannel(IProvisioningChannel &channel);

        /// @brief Inicia o processo de provisionamento.
        void begin();

        /// @brief Executa o ciclo de processamento de todos os canais ativos.
        void loop();

        /// @brief Interrompe todos os canais de provisionamento.
        void stop();

        /// @brief Indica se o dispositivo já foi provisionado nesta sessão.
        bool isProvisioned() const { return _isProvisioned; }

        /// @brief Define callback a ser chamado quando o provisionamento for concluído com sucesso.
        void onProvisioningCompleted(ProvisioningCompletedCallback cb) { _completedCb = cb; }

        /// @brief Define callback para notificação de mudanças de status dos canais.
        void onChannelStatus(ChannelStatusCallback cb) { _statusCb = cb; }

    private:
        std::vector<IProvisioningChannel *> _channels;
        bool _isProvisioned = false;
        ProvisioningCompletedCallback _completedCb = nullptr;
        ChannelStatusCallback _statusCb = nullptr;
        ILogger *_logger = nullptr;

        void handleNewConfig(const DeviceConfig &cfg, IProvisioningChannel *source);
        void handleChannelStatusInternal(IProvisioningChannel *channel, ProvisioningStatus status, const char *message);
        void startHighestPriorityChannels();
        void stopAllChannels();
    };

} // namespace iotsmartsys::core::provisioning