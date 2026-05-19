#include "App/Managers/DeviceRegistrationManager.h"

#include <Arduino.h>

#if defined(ESP32) && __has_include("esp_http_client.h")
#define IOTSMARTSYS_HAS_ESP_HTTP_CLIENT 1
#include "Infra/Certs/GoogleTrustServicesGTSRootR4.h"
extern "C"
{
#include "esp_err.h"
#include "esp_http_client.h"
}
#else
#define IOTSMARTSYS_HAS_ESP_HTTP_CLIENT 0
#endif

namespace iotsmartsys::app
{
    namespace
    {
        constexpr uint32_t kInitialRetryMs = 5000;
        constexpr uint32_t kMaxRetryMs = 60000;
        constexpr uint32_t kHttpTimeoutMs = 7000;
        constexpr const char *kFixedLastActive = "14/02/2026 20:17:23";

        bool endsWith(const std::string &value, const std::string &suffix)
        {
            return value.size() >= suffix.size() &&
                   value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        void trimTrailingSlashes(std::string &value)
        {
            while (!value.empty() && value.back() == '/')
            {
                value.pop_back();
            }
        }
    }

    struct DeviceRegistrationManager::RegistrationTaskContext
    {
        DeviceRegistrationManager *self{nullptr};
        core::settings::Settings settings{};
        std::string deviceId{};
    };

    DeviceRegistrationManager::DeviceRegistrationManager(core::ILogger &logger,
                                                         core::settings::SettingsManager &settingsManager,
                                                         core::WiFiManager &wifi,
                                                         core::IDeviceIdentityProvider &deviceIdentityProvider)
        : logger_(logger),
          settingsManager_(settingsManager),
          wifi_(wifi),
          deviceIdentityProvider_(deviceIdentityProvider)
    {
    }

    void DeviceRegistrationManager::handle()
    {
        completeRegistrationIfReady();

        if (registered_ || !wifi_.isConnected())
        {
            return;
        }

        if (registrationTaskRunning_)
        {
            return;
        }

        const uint32_t nowMs = millis();
        if (nowMs < nextAttemptAtMs_)
        {
            return;
        }

        core::settings::Settings settings;
        if (!settingsManager_.copyCurrent(settings))
        {
            scheduleRetry(nowMs);
            return;
        }

        if (settings.device_registered)
        {
            if (settings.mqtt.isValid())
            {
                registered_ = true;
                if (!cachedRegisteredLogged_)
                {
                    logger_.info("DeviceRegistration", "Device already marked as registered in cache.");
                    cachedRegisteredLogged_ = true;
                }
                return;
            }

            logger_.warn("DeviceRegistration", "Cached registration flag is set, but MQTT settings are invalid. Registration will be retried.");
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

        startRegistrationTask(settings, deviceId);
    }

    void DeviceRegistrationManager::completeRegistrationIfReady()
    {
        if (!registrationTaskCompleted_.exchange(false, std::memory_order_acq_rel))
        {
            return;
        }

        registrationTaskRunning_ = false;
        const bool succeeded = registrationTaskSucceeded_.load(std::memory_order_acquire);
        if (succeeded)
        {
            if (markRegisteredInCache(registrationSettingsSnapshot_))
            {
                registered_ = true;
                failures_ = 0;
                logger_.info("DeviceRegistration", "Device registered successfully.");
                return;
            }
            logger_.warn("DeviceRegistration", "Registration succeeded but could not persist registration flag.");
        }

        scheduleRetry(millis());
    }

    void DeviceRegistrationManager::startRegistrationTask(const core::settings::Settings &settings, const std::string &deviceId)
    {
#if IOTSMARTSYS_HAS_ESP_HTTP_CLIENT
        auto *ctx = new RegistrationTaskContext();
        if (!ctx)
        {
            scheduleRetry(millis());
            return;
        }

        ctx->self = this;
        ctx->settings = settings;
        ctx->deviceId = deviceId;
        registrationSettingsSnapshot_ = settings;
        registrationTaskSucceeded_.store(false, std::memory_order_release);
        registrationTaskCompleted_.store(false, std::memory_order_release);

        BaseType_t ok = xTaskCreate(&DeviceRegistrationManager::registrationTaskEntry,
                                    "device_register",
                                    6144,
                                    ctx,
                                    4,
                                    nullptr);
        if (ok == pdPASS)
        {
            registrationTaskRunning_ = true;
            return;
        }

        delete ctx;
        logger_.warn("DeviceRegistration", "Registration task creation failed. Retrying later.");
        scheduleRetry(millis());
#else
        (void)settings;
        (void)deviceId;
        if (!missingHttpClientLogged_)
        {
            logger_.warn("DeviceRegistration", "esp_http_client is not available in this build. Device registration disabled.");
            missingHttpClientLogged_ = true;
        }
        scheduleRetry(millis());
#endif
    }

    void DeviceRegistrationManager::registrationTaskEntry(void *arg)
    {
        auto *ctx = static_cast<RegistrationTaskContext *>(arg);
        if (!ctx || !ctx->self)
        {
            delete ctx;
            vTaskDelete(nullptr);
            return;
        }

        DeviceRegistrationManager *self = ctx->self;
        const bool succeeded = self->tryRegister(ctx->settings, ctx->deviceId);
        self->registrationTaskSucceeded_.store(succeeded, std::memory_order_release);
        self->registrationTaskCompleted_.store(true, std::memory_order_release);
        delete ctx;
        vTaskDelete(nullptr);
    }

    bool DeviceRegistrationManager::tryRegister(const core::settings::Settings &settings, const std::string &deviceId)
    {
#if IOTSMARTSYS_HAS_ESP_HTTP_CLIENT
        const std::string registrationUrl = resolveRegistrationUrl(settings.api.url);
        if (registrationUrl.empty())
        {
            logger_.warn("DeviceRegistration", "Could not resolve registration URL from API URL.");
            return false;
        }

        logger_.info("DeviceRegistration", "Attempting device registration. url='%s' deviceId='%s'.",
                     registrationUrl.c_str(),
                     deviceId.c_str());

        const std::string payload = buildPayload(
            deviceId,
            wifi_.getMacAddress() ? wifi_.getMacAddress() : "",
            wifi_.getIpAddress() ? wifi_.getIpAddress() : "");

        const bool useHttps = registrationUrl.rfind("https://", 0) == 0;

        esp_http_client_config_t config = {};
        config.url = registrationUrl.c_str();
        config.method = HTTP_METHOD_POST;
        config.timeout_ms = kHttpTimeoutMs;
        if (useHttps)
        {
            config.cert_pem = GTS_ROOT_R4_PEM;
            config.skip_cert_common_name_check = false;
        }

        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (!client)
        {
            logger_.warn("DeviceRegistration", "Failed to initialize esp_http_client for URL: %s", registrationUrl.c_str());
            return false;
        }

        esp_http_client_set_header(client, "Content-Type", "application/json");
        esp_http_client_set_header(client, "x-api-key", settings.api.key.c_str());
        esp_http_client_set_header(client, "Authorization", settings.api.basic_auth.c_str());
        esp_http_client_set_post_field(client, payload.c_str(), static_cast<int>(payload.length()));

        const esp_err_t err = esp_http_client_perform(client);
        const int statusCode = esp_http_client_get_status_code(client);
        esp_http_client_cleanup(client);

        if (err != ESP_OK)
        {
            logger_.warn("DeviceRegistration", "Registration request failed. err=%s(%d) HTTP %d.",
                         esp_err_to_name(err),
                         static_cast<int>(err),
                         statusCode);
            return false;
        }

        if (statusCode == 201 || statusCode == 409)
        {
            logger_.info("DeviceRegistration", "Registration endpoint returned HTTP %d.", statusCode);
            return true;
        }

        logger_.warn("DeviceRegistration", "Registration failed. HTTP %d.", statusCode);
        return false;
#else
        (void)settings;
        (void)deviceId;
        if (!missingHttpClientLogged_)
        {
            logger_.warn("DeviceRegistration", "esp_http_client is not available in this build. Device registration disabled.");
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

        trimTrailingSlashes(resolved);

        if (endsWith(resolved, "/devices/:device_id"))
        {
            resolved.erase(resolved.size() - std::string("/devices/:device_id").size());
            trimTrailingSlashes(resolved);
        }

        if (!endsWith(resolved, "/devices"))
        {
            resolved += "/devices";
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

    bool DeviceRegistrationManager::markRegisteredInCache(const core::settings::Settings &settingsSnapshot)
    {
        core::settings::Settings persisted = settingsSnapshot;
        persisted.device_registered = true;
        return settingsManager_.save(persisted);
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
