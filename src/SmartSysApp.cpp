#include "SmartSysApp.h"
#include "App/Builders/Builders/AnnouncePayloadBuilder.h"
#include "Version/VersionInfo.h"
#if IOTSMARTSYS_OTA_ENABLED
#include "Infra/OTA/OTA.h"
#include "Infra/OTA/OTAManager.h"
#include "Platform/Espressif/Parsers/EspIdFirmwareManifestParser.h"
#endif

#include <cmath>
#include <cstdio>
#include <memory>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "esp_freertos_hooks.h"
namespace iotsmartsys
{
    namespace
    {
        constexpr uint32_t kNetworkTaskIntervalMs = 50;
        constexpr uint32_t kTransportTaskIntervalMs = 20;
        constexpr uint32_t kNetworkTaskStackWords = 4096;
        constexpr uint32_t kTransportTaskStackWords = 6144;
        constexpr UBaseType_t kNetworkTaskPriority = 4;
        constexpr UBaseType_t kTransportTaskPriority = 4;
        constexpr const char *kMetricsTopic = "device/metrics";
        volatile uint32_t gCpuTotalTicks[portNUM_PROCESSORS]{};
        volatile uint32_t gCpuIdleTicks[portNUM_PROCESSORS]{};
        TaskHandle_t gIdleTaskHandles[portNUM_PROCESSORS]{};

        void IRAM_ATTR cpuUsageTickHook()
        {
            const BaseType_t core = xPortGetCoreID();
            if (core < 0 || core >= portNUM_PROCESSORS)
            {
                return;
            }

            gCpuTotalTicks[core]++;
            if (xTaskGetCurrentTaskHandleForCPU(core) == gIdleTaskHandles[core])
            {
                gCpuIdleTicks[core]++;
            }
        }

        void appendJsonString(std::string &out, const char *value)
        {
            out.push_back('"');
            if (value)
            {
                for (const char *p = value; *p; ++p)
                {
                    switch (*p)
                    {
                    case '"':
                        out += "\\\"";
                        break;
                    case '\\':
                        out += "\\\\";
                        break;
                    case '\n':
                        out += "\\n";
                        break;
                    case '\r':
                        out += "\\r";
                        break;
                    case '\t':
                        out += "\\t";
                        break;
                    default:
                        out.push_back(*p);
                        break;
                    }
                }
            }
            out.push_back('"');
        }
    }

    SmartSysApp::SmartSysApp()
        : serviceManager_(core::ServiceManager::init()),
          logger_(serviceManager_.logger()),
          settingsManager_(serviceManager_.settingsManager()),
          settingsGate_(serviceManager_.settingsGate()),
          commandParser_(logger_),
          mqttClient_(std::make_unique<platform::espressif::EspIdfMqttClient>(logger_)),
          mqtt_(*mqttClient_, logger_, settingsGate_, settingsManager_),
          mqttSink_(mqtt_, settingsManager_),
          hwFactory_(),
          deviceIdentityProvider_(),
          wifi_(logger_),
          provisioningController_(logger_, wifi_, deviceIdentityProvider_),
          deviceStateManager_(logger_, hwFactory_, wifi_, provisioningController_),
          deviceRegistrationManager_(logger_, settingsManager_, wifi_, deviceIdentityProvider_),
          connectivityBootstrap_(logger_, serviceManager_, settingsManager_, wifi_),
          builder_(hwFactory_,
                   mqttSink_,
                   capSlots_,
                   capDtors_,
                   8,
                   adapterSlots_,
                   adapterDtors_,
                   8,
                   arena_,
                   sizeof(arena_),
                   deviceIdentityProvider_),
          systemCommandProcessor_(logger_),
          factoryResetButtonController_(logger_, settingsManager_, systemCommandProcessor_, hwFactory_),
          capabilityController_(logger_, commandParser_, systemCommandProcessor_),
          transportController_(logger_, settingsManager_, mqtt_),
          uart_(nullptr)
    {
#if IOTSMARTSYS_OTA_ENABLED
        manifestParser_ = std::make_unique<platform::espressif::ota::EspIdFirmwareManifestParser>();
#ifndef OTA_DISABLED
        ota_ = std::make_unique<ota::OTA>(logger_, deviceIdentityProvider_);
#endif
        otaManager_ = std::make_unique<ota::OTAManager>(
            settingsManager_,
            logger_,
            *manifestParser_,
#ifndef OTA_DISABLED
            *ota_,
#endif
            settingsGate_);
#endif
    }

    SmartSysApp::~SmartSysApp() = default;

    void SmartSysApp::configureFactoryResetButton(iotsmartsys::app::PushButtonConfig cfg)
    {
        factoryResetButtonController_.configure(cfg);
    }

    void SmartSysApp::configureLED(iotsmartsys::app::LightConfig cfg)
    {
        deviceStateManager_.configure(cfg);
    }

    void SmartSysApp::configureSerialTransport(HardwareSerial &serial, uint32_t baudRate, int rxPin, int txPin)
    {
        if (uart_)
        {
            delete uart_;
            uart_ = nullptr;
        }
        uart_ = new core::SerialTransportChannel(serial, baudRate, rxPin, txPin);
        TransportConfig cfg{};
        cfg.uri = "serial://uart2";
        cfg.clientId = "esp32-uart-bridge";
        cfg.keepAliveSec = 30;
        uart_->begin(cfg);
        uart_->setForwardRawMessages(true);
        transportController_.addChannel("uart", uart_);
    }

    void SmartSysApp::applySettingsToRuntime(const iotsmartsys::core::settings::Settings &)
    {
        logger_.warn("SettingsManager", "Runtime settings changed; restarting device to apply updated configuration.");
        mqtt_.stop();
        delay(250);
        systemCommandProcessor_.restartSafely();
    }

    void SmartSysApp::onMqttConnected(const core::TransportConnectedView &info)
    {
        auto *capabilityManager = capabilityController_.manager();
        if (!capabilityManager)
        {
            return;
        }
        app::AnnouncePayloadBuilder builder(
            capabilityManager->getAllCapabilities(), logger_);

        const auto mqttQueueStats = mqtt_.offlineQueueStats();
        const auto mqttReconnectStats = mqtt_.reconnectStats();

        builder.withDeviceId(info.clientId)
            .withBroker(info.broker)
            .withVersion(IOTSMARTSYSCORE_VERSION)
            .withBuild(getBuildIdentifier())
            .withProperty(Property("wifi_ssid", wifi_.getSsid()))
            .withProperty(Property("wifi_signal", wifi_.getSignalStrength()))
            .withIpAddress(wifi_.getIpAddress())
            .withMacAddress(wifi_.getMacAddress());
        // .withChipModel(wifi_.getChipModel());

        std::string payload = builder.build();

        core::settings::Settings currentSettings;
        if (!settingsManager_.copyCurrent(currentSettings))
        {
            return;
        }

        std::string topic = currentSettings.mqtt.announce_topic;
        mqtt_.publish(topic.c_str(), payload.c_str(), payload.length(), false);
    }

    void SmartSysApp::onSettingsUpdated(const iotsmartsys::core::settings::Settings &newSettings)
    {
        if (newSettings.hasChanges())
        {
            settings_ = newSettings;
            applySettingsToRuntime(newSettings);
            return;
        }

        logger_.info("SettingsManager", "Settings update callback invoked with no runtime changes.");
    }

    void SmartSysApp::setup()
    {
        serviceManager_.setLogLevel(core::LogLevel::Info);
        core::Log::setLogger(&logger_);
        delay(3000);
        logger_.info("---------------------------------------------------------");
        logger_.info("App", "IoT SmartSys Core Version: %s", IOTSMARTSYSCORE_VERSION);
        logger_.info("App", "Device ID: %s", deviceIdentityProvider_.getDeviceID().c_str());
        logger_.info("---------------------------------------------------------");

        registerCpuUsageHooks();
        settingsManager_.setUpdatedCallback(SmartSysApp::onSettingsUpdatedThunk, this);
        iotsmartsys::core::ConnectivityGate::init(latch_);

        const auto path = connectivityBootstrap_.run(settings_);
        if (path == app::ConnectivityBootstrap::BootPath::Provisioning)
        {
            provisioningController_.begin();
            return;
        }

        capabilityController_.setup(builder_);
        transportController_.addDispatcher(*capabilityController_.dispatcher());
        transportController_.configureMqtt(
            deviceIdentityProvider_.getDeviceID().c_str(),
            &SmartSysApp::onMqttConnectedThunk,
            this);
        startRuntimeTasks();
    }

    void SmartSysApp::handle()
    {
        if (!networkTaskStarted_)
        {
            wifi_.handle();
        }

        deviceStateManager_.handle();
        if (provisioningController_.isActive())
        {
            provisioningController_.handle();
            return;
        }

        factoryResetButtonController_.handle();

        if (!transportTaskStarted_)
        {
            handleTransportWork();
        }

        capabilityController_.handle();
#if IOTSMARTSYS_OTA_ENABLED
        if (otaManager_)
        {
            otaManager_->handle();
        }
#endif
    }

    bool SmartSysApp::hasOperationalMqttConfig() const
    {
        core::settings::Settings currentSettings;
        if (!settingsManager_.copyCurrent(currentSettings))
        {
            return false;
        }

        return currentSettings.isValidWifiConfig() && currentSettings.mqtt.isValid();
    }

    void SmartSysApp::startRuntimeTasks()
    {
        if (runtimeTasksEnabled_.load(std::memory_order_acquire))
        {
            return;
        }

        runtimeTasksEnabled_.store(true, std::memory_order_release);

        BaseType_t networkOk = xTaskCreate(&SmartSysApp::networkTaskEntry,
                                           "iot_network",
                                           kNetworkTaskStackWords,
                                           this,
                                           kNetworkTaskPriority,
                                           &networkTask_);
        if (networkOk == pdPASS)
        {
            networkTaskStarted_ = true;
        }
        else
        {
            networkTask_ = nullptr;
            logger_.warn("Runtime", "Network task creation failed. WiFi will run in main loop.");
        }

        BaseType_t transportOk = xTaskCreate(&SmartSysApp::transportTaskEntry,
                                             "iot_transport",
                                             kTransportTaskStackWords,
                                             this,
                                             kTransportTaskPriority,
                                             &transportTask_);
        if (transportOk == pdPASS)
        {
            transportTaskStarted_ = true;
        }
        else
        {
            transportTask_ = nullptr;
            logger_.warn("Runtime", "Transport task creation failed. MQTT/settings will run in main loop.");
        }

        if (!networkTaskStarted_ && !transportTaskStarted_)
        {
            runtimeTasksEnabled_.store(false, std::memory_order_release);
        }
    }

    void SmartSysApp::handleTransportWork()
    {
        settingsManager_.handle();

        // Avoid overlapping TLS handshakes on constrained ESP32 heaps.
        // The settings refresh must finish before registration or MQTT starts.
        if (settingsManager_.state() == core::settings::SettingsManagerState::FetchingFromApi)
        {
            return;
        }

        if (wifi_.isConnected() && !deviceRegistrationManager_.isRegistered())
        {
            deviceRegistrationManager_.handle();
        }

        if (!deviceRegistrationManager_.isRegistered() && !hasOperationalMqttConfig())
        {
            return;
        }

        if (!transportStarted_)
        {
            transportController_.start();
            transportStarted_ = true;
        }

        transportController_.handle();
        publishMetricsIfDue();
    }

    void SmartSysApp::publishMetricsIfDue()
    {
        core::settings::Settings currentSettings;
        if (!settingsManager_.copyCurrent(currentSettings))
        {
            return;
        }

        if (currentSettings.collect_interval_metrics <= 0)
        {
            return;
        }

        const uint32_t now = millis();
        const uint32_t intervalMs = static_cast<uint32_t>(currentSettings.collect_interval_metrics);
        if (lastMetricsPublishAtMs_ != 0 && static_cast<uint32_t>(now - lastMetricsPublishAtMs_) < intervalMs)
        {
            return;
        }

        if (!mqtt_.isConnected())
        {
            return;
        }

        publishMetrics(currentSettings);
        lastMetricsPublishAtMs_ = now;
    }

    void SmartSysApp::publishMetrics(const core::settings::Settings &settings)
    {
        const std::string payload = buildMetricsPayload(settings);
        mqtt_.publish(kMetricsTopic, payload.c_str(), payload.length(), false);
    }

    std::string SmartSysApp::buildMetricsPayload(const core::settings::Settings &settings)
    {
        (void)settings;
        std::string payload;
        payload.reserve(320);

        payload += "{\"device_id\":";
        appendJsonString(payload, deviceIdentityProvider_.getDeviceID().c_str());
        payload += ",\"uptime_ms\":";
        payload += std::to_string(millis());
        payload += ",\"cpu_cores\":";
        payload += std::to_string(ESP.getChipCores());
        payload += ",\"cpu_percent\":";
        char cpuUsageBuf[16];
        std::snprintf(cpuUsageBuf, sizeof(cpuUsageBuf), "%.1f", calculateCpuPercentUsage());
        payload += cpuUsageBuf;

        const uint32_t heapFree = ESP.getFreeHeap();
        const uint32_t heapSize = ESP.getHeapSize();
        const float heapPercentUsage = heapSize > 0
                                           ? (100.0f * static_cast<float>(heapSize - heapFree) / static_cast<float>(heapSize))
                                           : 0.0f;

        payload += ",\"memory_percent\":";
        char percentBuf[16];
        std::snprintf(percentBuf, sizeof(percentBuf), "%.1f", heapPercentUsage);
        payload += percentBuf;
        payload += ",\"temperature_c\":";

        const float temperature = temperatureRead();
        if (std::isnan(temperature))
        {
            payload += "null";
        }
        else
        {
            char tempBuf[16];
            std::snprintf(tempBuf, sizeof(tempBuf), "%.2f", temperature);
            payload += tempBuf;
        }

        payload += ",\"frequency_mhz\":";
        payload += std::to_string(ESP.getCpuFreqMHz());
        payload += ",\"network\":{\"rssi\":";
        payload += std::to_string(wifi_.getRssi());
        payload += ",\"last_desconnected\":";
        payload += std::to_string(wifi_.getLastDisconnectedAtMs());
        payload += ",\"desconnected_rason\":";
        payload += std::to_string(wifi_.getLastDisconnectReason());
        payload += ",\"connection_count\":";
        payload += std::to_string(wifi_.getConnectionCount());
        payload += "}}";

        return payload;
    }

    void SmartSysApp::registerCpuUsageHooks()
    {
        if (cpuUsageHooksRegistered_)
        {
            return;
        }

        bool allRegistered = true;
        for (UBaseType_t core = 0; core < portNUM_PROCESSORS; ++core)
        {
            gIdleTaskHandles[core] = xTaskGetIdleTaskHandleForCPU(core);
            const esp_err_t err = esp_register_freertos_tick_hook_for_cpu(cpuUsageTickHook, core);
            if (err != ESP_OK)
            {
                allRegistered = false;
                logger_.warn("Metrics", "Failed to register CPU usage tick hook for core %u: %d", (unsigned)core, (int)err);
            }
        }

        cpuUsageHooksRegistered_ = allRegistered;
    }

    float SmartSysApp::calculateCpuPercentUsage()
    {
        uint32_t totalRunTime = 0;
        uint32_t idleRunTime = 0;
        for (UBaseType_t core = 0; core < portNUM_PROCESSORS; ++core)
        {
            totalRunTime += gCpuTotalTicks[core];
            idleRunTime += gCpuIdleTicks[core];
        }

        if (totalRunTime == 0)
        {
            return 0.0f;
        }

        if (!hasCpuSample_)
        {
            lastCpuTotalRunTime_ = totalRunTime;
            lastCpuIdleRunTime_ = idleRunTime;
            hasCpuSample_ = true;
            return 0.0f;
        }

        const uint32_t totalDelta = totalRunTime - lastCpuTotalRunTime_;
        const uint32_t idleDelta = idleRunTime - lastCpuIdleRunTime_;
        lastCpuTotalRunTime_ = totalRunTime;
        lastCpuIdleRunTime_ = idleRunTime;

        if (totalDelta == 0)
        {
            return 0.0f;
        }

        const float idlePercent = 100.0f * static_cast<float>(idleDelta) / static_cast<float>(totalDelta);
        float usagePercent = 100.0f - idlePercent;
        if (usagePercent < 0.0f)
        {
            usagePercent = 0.0f;
        }
        else if (usagePercent > 100.0f)
        {
            usagePercent = 100.0f;
        }
        return usagePercent;
    }

    void SmartSysApp::networkTaskLoop()
    {
        while (runtimeTasksEnabled_.load(std::memory_order_acquire))
        {
            if (!provisioningController_.isActive())
            {
                wifi_.handle();
            }
            vTaskDelay(pdMS_TO_TICKS(kNetworkTaskIntervalMs));
        }

        networkTaskStarted_ = false;
        networkTask_ = nullptr;
    }

    void SmartSysApp::transportTaskLoop()
    {
        while (runtimeTasksEnabled_.load(std::memory_order_acquire))
        {
            if (!provisioningController_.isActive())
            {
                handleTransportWork();
            }
            vTaskDelay(pdMS_TO_TICKS(kTransportTaskIntervalMs));
        }

        transportTaskStarted_ = false;
        transportTask_ = nullptr;
    }

    void SmartSysApp::networkTaskEntry(void *ctx)
    {
        auto *self = static_cast<SmartSysApp *>(ctx);
        if (self)
        {
            self->networkTaskLoop();
        }
        vTaskDelete(nullptr);
    }

    void SmartSysApp::transportTaskEntry(void *ctx)
    {
        auto *self = static_cast<SmartSysApp *>(ctx);
        if (self)
        {
            self->transportTaskLoop();
        }
        vTaskDelete(nullptr);
    }

    void SmartSysApp::onMqttConnectedThunk(void *ctx, const core::TransportConnectedView &info)
    {
        if (!ctx)
        {
            return;
        }
        static_cast<SmartSysApp *>(ctx)->onMqttConnected(info);
    }

    void SmartSysApp::onSettingsUpdatedThunk(const core::settings::Settings &newSettings, void *ctx)
    {
        if (!ctx)
        {
            return;
        }
        static_cast<SmartSysApp *>(ctx)->onSettingsUpdated(newSettings);
    }

    /// @brief Adds a new alarm capability to the application.
    iotsmartsys::core::AlarmCapability *SmartSysApp::addAlarmCapability(iotsmartsys::app::AlarmConfig cfg)
    {
        return builder_.addAlarm(cfg);
    }

    /// @brief Adds a new door sensor capability to the application.
    iotsmartsys::core::DoorSensorCapability *SmartSysApp::addDoorSensorCapability(iotsmartsys::app::DoorSensorConfig cfg)
    {
        return builder_.addDoorSensor(cfg);
    }

    /// @brief Adds a new clap sensor capability to the application.
    iotsmartsys::core::ClapSensorCapability *SmartSysApp::addClapSensorCapability(iotsmartsys::app::ClapSensorConfig cfg)
    {
        return builder_.addClapSensor(cfg);
    }

    /// @brief Adds a new light capability to the application.
    iotsmartsys::core::LightCapability *SmartSysApp::addLightCapability(iotsmartsys::app::LightConfig cfg)
    {
        return builder_.addLight(cfg);
    }

    /// @brief Adds a new GLP sensor capability to the application.
    iotsmartsys::core::GlpSensorCapability *SmartSysApp::addGlpSensorCapability(iotsmartsys::app::GlpSensorConfig cfg)
    {
        return builder_.addGlpSensor(cfg);
    }

    /// @brief Adds a new GLP meter Percent capability to the application.
    iotsmartsys::core::GlpMeterPercentCapability *SmartSysApp::addGlpMeterPercentCapability(iotsmartsys::app::GlpMeterConfig cfg)
    {
        return builder_.addGlpMeterPercent(cfg);
    }

    /// @brief Adds a new GLP meter Kg capability to the application.
    iotsmartsys::core::GlpMeterKgCapability *SmartSysApp::addGlpMeterKgCapability(iotsmartsys::app::GlpMeterConfig cfg)
    {
        return builder_.addGlpMeterKg(cfg);
    }

    /// @brief Adds a new humidity sensor capability to the application.
    iotsmartsys::core::HumiditySensorCapability *SmartSysApp::addHumiditySensorCapability(iotsmartsys::app::HumiditySensorConfig cfg)
    {
        return builder_.addHumiditySensor(cfg);
    }

    /// @brief Adds a new distance sensor capability to the application.
    iotsmartsys::core::DistanceCapability *SmartSysApp::addDistanceCapability(iotsmartsys::app::DistanceCapabilityConfig cfg)
    {
        return builder_.addDistance(cfg);
    }

    /// @brief Adds a new LED capability to the application.
    iotsmartsys::core::LEDCapability *SmartSysApp::addLedCapability(iotsmartsys::app::LightConfig cfg)
    {
        return builder_.addLED(cfg);
    }

    /// @brief Adds a new PIR sensor capability to the application.
    iotsmartsys::core::PirSensorCapability *SmartSysApp::addPirSensorCapability(iotsmartsys::app::PirSensorConfig cfg)
    {
        return builder_.addPirSensor(cfg);
    }

    /// @brief Adds a new push button capability to the application.
    iotsmartsys::core::PushButtonCapability *SmartSysApp::addPushButtonCapability(iotsmartsys::app::PushButtonConfig cfg)
    {
        return builder_.addPushButton(cfg);
    }

    /// @brief Adds a new touch button capability to the application.
    iotsmartsys::core::TouchButtonCapability *SmartSysApp::addTouchButtonCapability(iotsmartsys::app::TouchButtonConfig cfg)
    {
        return builder_.addTouchButton(cfg);
    }

    /// @brief Adds a new switch capability to the application.
    iotsmartsys::core::SwitchCapability *SmartSysApp::addSwitchCapability(iotsmartsys::app::SwitchConfig cfg)
    {
        return builder_.addSwitch(cfg);
    }

    /// @brief Adds a new switch plug capability to the application.
    iotsmartsys::core::SwitchPlugCapability *SmartSysApp::addSwitchPlugCapability(iotsmartsys::app::SwitchConfig cfg)
    {
        return builder_.addSwitchPlug(cfg);
    }

    /// @brief Adds a new valve capability to the application.
    iotsmartsys::core::ValveCapability *SmartSysApp::addValveCapability(iotsmartsys::app::ValveConfig cfg)
    {
        return builder_.addValve(cfg);
    }

    /// @brief Adds a new operational color sensor capability to the application.
    iotsmartsys::core::OperationalColorSensorCapability *SmartSysApp::addOperationalColorSensorCapability(iotsmartsys::app::OperationalColorSensorConfig cfg)
    {
        return builder_.addOperationalColorSensor(cfg);
    }

    /// @brief Adds a new temperature sensor capability to the application.
    iotsmartsys::core::TemperatureSensorCapability *SmartSysApp::addTemperatureSensorCapability(iotsmartsys::app::TemperatureSensorConfig cfg)
    {
        return builder_.addTemperatureSensor(cfg);
    }

    /// @brief Adds a new water level liters capability to the application.
    iotsmartsys::core::WaterLevelLitersCapability *SmartSysApp::addWaterLevelLitersCapability(iotsmartsys::app::WaterLevelSensorConfig cfg)
    {
        return builder_.addWaterLevelLiters(cfg);
    }

    /// @brief Adds a new water level percent capability to the application.
    iotsmartsys::core::WaterLevelPercentCapability *SmartSysApp::addWaterLevelPercentCapability(iotsmartsys::app::WaterLevelSensorConfig cfg)
    {
        return builder_.addWaterLevelPercent(cfg);
    }

    /// @brief Adds a new water flow hall sensor capability to the application.
    iotsmartsys::core::WaterFlowHallSensorCapability *SmartSysApp::addWaterFlowHallSensorCapability(iotsmartsys::app::WaterFlowHallSensorConfig cfg)
    {
        return builder_.addWaterFlowHallSensor(cfg);
    }

    /// @brief Adds a new luminosity sensor capability to the application.
    iotsmartsys::core::LuminosityCapability *SmartSysApp::addLuminosityCapability(iotsmartsys::app::LuminositySensorConfig cfg)
    {
        return builder_.addLuminosityCapability(cfg);
    }

    /// @brief Adds a new garage control capability to the application.
    iotsmartsys::core::GarageControlCapability *SmartSysApp::addGarageControlCapability(iotsmartsys::app::GarageControlConfig cfg)
    {
        return builder_.addGarageControlCapability(cfg);
    }

} // namespace iotsmartsys
