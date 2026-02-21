#include "App/Managers/DeviceRegistrationManager.h"

#include <Arduino.h>

#if __has_include(<HTTPClient.h>)
#define IOTSMARTSYS_HAS_HTTP_CLIENT 1
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#else
#define IOTSMARTSYS_HAS_HTTP_CLIENT 0
#endif

namespace iotsmartsys::app
{
    namespace
    {
        constexpr uint32_t kInitialRetryMs = 5000;
        constexpr uint32_t kMaxRetryMs = 60000;
        constexpr uint32_t kHttpTimeoutMs = 7000;
        constexpr const char *kFixedLastActive = "14/02/2026 20:17:23";
    }

    DeviceRegistrationManager::DeviceRegistrationManager(core::ILogger &logger,
                                                         core::settings::IReadOnlySettingsProvider &settingsProvider,
                                                         core::WiFiManager &wifi,
                                                         core::IDeviceIdentityProvider &deviceIdentityProvider)
        : logger_(logger),
          settingsProvider_(settingsProvider),
          wifi_(wifi),
          deviceIdentityProvider_(deviceIdentityProvider)
    {
    }

    void DeviceRegistrationManager::handle()
    {
        if (registered_ || !wifi_.isConnected())
        {
            return;
        }

        const uint32_t nowMs = millis();
        if (nowMs < nextAttemptAtMs_)
        {
            return;
        }

        core::settings::Settings settings;
        if (!settingsProvider_.copyCurrent(settings))
        {
            scheduleRetry(nowMs);
            return;
        }

        if (!settings.api.isValid())
        {
            if (!invalidApiLogged_)
            {
                logger_.warn("DeviceRegistration", "API config is invalid. Device registration skipped until settings are updated.");
                invalidApiLogged_ = true;
            }
            scheduleRetry(nowMs);
            return;
        }

        invalidApiLogged_ = false;

        const std::string deviceId = deviceIdentityProvider_.getDeviceID();
        if (deviceId.empty())
        {
            logger_.warn("DeviceRegistration", "Device ID is empty. Registration postponed.");
            scheduleRetry(nowMs);
            return;
        }

        if (tryRegister(settings, deviceId))
        {
            registered_ = true;
            logger_.info("DeviceRegistration", "Device registered successfully.");
            return;
        }

        scheduleRetry(nowMs);
    }

    bool DeviceRegistrationManager::tryRegister(const core::settings::Settings &settings, const std::string &deviceId)
    {
#if IOTSMARTSYS_HAS_HTTP_CLIENT
        const std::string registrationUrl = resolveRegistrationUrl(settings.api.url);
        if (registrationUrl.empty())
        {
            logger_.warn("DeviceRegistration", "Could not resolve registration URL from API URL.");
            return false;
        }

        const std::string payload = buildPayload(
            deviceId,
            wifi_.getMacAddress() ? wifi_.getMacAddress() : "",
            wifi_.getIpAddress() ? wifi_.getIpAddress() : "");

        HTTPClient http;
        http.setTimeout(kHttpTimeoutMs);

        const bool useHttps = registrationUrl.rfind("https://", 0) == 0;
        bool beginOk = false;

        WiFiClient httpClient;
        WiFiClientSecure httpsClient;

        if (useHttps)
        {
            httpsClient.setInsecure();
            beginOk = http.begin(httpsClient, String(registrationUrl.c_str()));
        }
        else
        {
            beginOk = http.begin(httpClient, String(registrationUrl.c_str()));
        }

        if (!beginOk)
        {
            logger_.warn("DeviceRegistration", "Failed to initialize HTTP client for URL: %s", registrationUrl.c_str());
            http.end();
            return false;
        }

        http.addHeader("Content-Type", "application/json");
        http.addHeader("x-api-key", settings.api.key.c_str());
        http.addHeader("Authorization", settings.api.basic_auth.c_str());

        const int statusCode = http.POST(String(payload.c_str()));
        const String response = http.getString();
        http.end();

        if (statusCode == 201 || statusCode == 409)
        {
            logger_.info("DeviceRegistration", "Registration endpoint returned HTTP %d.", statusCode);
            return true;
        }

        logger_.warn("DeviceRegistration", "Registration failed. HTTP %d, response: %s", statusCode, response.c_str());
        return false;
#else
        (void)settings;
        (void)deviceId;
        if (!missingHttpClientLogged_)
        {
            logger_.warn("DeviceRegistration", "HTTPClient is not available in this build. Device registration disabled.");
            missingHttpClientLogged_ = true;
        }
        return false;
#endif
    }

    std::string DeviceRegistrationManager::resolveRegistrationUrl(const std::string &apiUrl) const
    {
        if (apiUrl.empty())
        {
            return {};
        }

        std::string resolved = apiUrl;

        const std::string settingsSuffix = ":device_id/settings";
        const std::size_t suffixPos = resolved.find(settingsSuffix);
        if (suffixPos != std::string::npos)
        {
            resolved.erase(suffixPos);
        }

        const std::string simpleSuffix = "/settings";
        const std::size_t simpleSuffixPos = resolved.rfind(simpleSuffix);
        if (simpleSuffixPos != std::string::npos && simpleSuffixPos == (resolved.size() - simpleSuffix.size()))
        {
            resolved.erase(simpleSuffixPos);
        }

        while (!resolved.empty() && resolved.back() == '/')
        {
            resolved.pop_back();
        }

        return resolved;
    }

    std::string DeviceRegistrationManager::buildPayload(const std::string &deviceId,
                                                        const std::string &macAddress,
                                                        const std::string &ipAddress)
    {
        const std::string safeDeviceId = escapeJson(deviceId);
        const std::string safeMac = escapeJson(macAddress);
        const std::string safeIp = escapeJson(ipAddress);

        std::string payload;
        payload.reserve(512);
        payload += "{";
        payload += "\"device_id\":\"" + safeDeviceId + "\",";
        payload += "\"device_name\":\"" + safeDeviceId + "\",";
        payload += "\"description\":\"" + safeDeviceId + "\",";
        payload += "\"last_active\":\"" + std::string(kFixedLastActive) + "\",";
        payload += "\"state\":\"online\",";
        payload += "\"mac_address\":\"" + safeMac + "\",";
        payload += "\"ip_address\":\"" + safeIp + "\",";
        payload += "\"protocol\":\"MQTT\",";
        payload += "\"platform\":\"ESP32\",";
        payload += "\"capabilities\":[],";
        payload += "\"properties\":[]";
        payload += "}";
        return payload;
    }

    std::string DeviceRegistrationManager::escapeJson(const std::string &value)
    {
        std::string escaped;
        escaped.reserve(value.size());

        for (char c : value)
        {
            switch (c)
            {
            case '\\':
                escaped += "\\\\";
                break;
            case '\"':
                escaped += "\\\"";
                break;
            case '\n':
                escaped += "\\n";
                break;
            case '\r':
                escaped += "\\r";
                break;
            case '\t':
                escaped += "\\t";
                break;
            default:
                escaped += c;
                break;
            }
        }

        return escaped;
    }

    void DeviceRegistrationManager::scheduleRetry(uint32_t nowMs)
    {
        if (failures_ < 15)
        {
            ++failures_;
        }

        uint32_t delayMs = kInitialRetryMs;
        for (uint8_t i = 1; i < failures_; ++i)
        {
            if (delayMs >= (kMaxRetryMs / 2))
            {
                delayMs = kMaxRetryMs;
                break;
            }
            delayMs *= 2;
        }

        if (delayMs > kMaxRetryMs)
        {
            delayMs = kMaxRetryMs;
        }

        nextAttemptAtMs_ = nowMs + delayMs;
    }
} // namespace iotsmartsys::app
