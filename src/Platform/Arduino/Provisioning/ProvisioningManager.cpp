#include <Arduino.h>
#include "Core/Provisioning/ProvisioningManager.h"

namespace iotsmartsys::core::provisioning
{

    void ProvisioningManager::registerChannel(IProvisioningChannel &channel)
    {
        _channels.push_back(&channel);

        channel.onConfigReceived([this, &channel](const DeviceConfig &cfg)
                                 { handleNewConfig(cfg, &channel); });

        channel.onStatusChanged([this, &channel](ProvisioningStatus status, const char *message)
                                { handleChannelStatusInternal(&channel, status, message); });
    }

    void ProvisioningManager::begin()
    {
        _isProvisioned = false;
        startHighestPriorityChannels();
    }

    void ProvisioningManager::handle()
    {
        if (_restartPending &&
            static_cast<int32_t>(millis() - _restartAtMs) >= 0)
        {
            ESP.restart();
        }

        if (_isProvisioned)
        {
            return;
        }

        for (auto *ch : _channels)
        {
            if (ch && ch->isActive())
            {
                ch->loop();
            }
        }
    }

    void ProvisioningManager::stop()
    {
        stopAllChannels();
        _isProvisioned = false;
        _restartPending = false;
    }

    void ProvisioningManager::scheduleRestart(uint32_t delayMs)
    {
        _restartPending = true;
        _restartAtMs = millis() + delayMs;
    }

    void ProvisioningManager::handleNewConfig(const DeviceConfig &cfg, IProvisioningChannel *source)
    {
        if (_isProvisioned)
        {
            return;
        }

        _isProvisioned = true;

        if (_statusCb && source)
        {
            _statusCb(*source, ProvisioningStatus::Applied, "ProvisioningManager: configuracao recebida");
        }

        if (_completedCb)
        {
            _completedCb(cfg);
        }

        stopAllChannels();
    }

    void ProvisioningManager::handleChannelStatusInternal(IProvisioningChannel *channel,
                                                          ProvisioningStatus status,
                                                          const char *message)
    {
        if (_statusCb && channel)
        {
            _statusCb(*channel, status, message);
        }

        if (message)
        {
            if (_logger)
           _logger->info(message);
        }
    }

    void ProvisioningManager::startHighestPriorityChannels()
    {
        if (_channels.empty())
        {
            return;
        }

        uint8_t maxPriority = 0;
        for (auto *ch : _channels)
        {
            if (ch)
            {
                uint8_t p = ch->priority();
                if (p > maxPriority)
                {
                    maxPriority = p;
                }
            }
        }

        for (auto *ch : _channels)
        {
            if (ch && ch->priority() == maxPriority)
            {
                ch->begin();
            }
        }
    }

    void ProvisioningManager::stopAllChannels()
    {
        for (auto *ch : _channels)
        {
            if (ch && ch->isActive())
            {
                ch->stop();
            }
        }
    }

} // namespace iotsmartsys::core::provisioning
