#pragma once

#include <string>

namespace iotsmartsys::core::settings
{

    struct MqttConfig
    {
        std::string host;
        int port{1883};
        std::string user;
        std::string password;
        std::string protocol;
        int ttl{0};
        uint16_t keepAliveSec{30};
        bool cleanSession{true};

        unsigned long getReconnectIntervalMs() const
        {
            return ttl * 60000UL;
        }

        bool isValid() const
        {
            return !host.empty() && port > 0 && port <= 65535;
        }

        bool hasChanged(const MqttConfig &other) const
        {
            return (host != other.host || port != other.port ||
                    user != other.user || password != other.password ||
                    protocol != other.protocol || ttl != other.ttl ||
                    keepAliveSec != other.keepAliveSec ||
                    cleanSession != other.cleanSession);
        }
    };

} // namespace iotsmartsys::core::settings