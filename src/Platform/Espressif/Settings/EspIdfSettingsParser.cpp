
#include "EspIdfSettingsParser.h"

#include <cstring>

#include "Contracts/Common/StateResult.h"
#include "Platform/Common/Json/JsonPathExtractor.h"

namespace iotsmartsys::platform::espressif
{
    using namespace iotsmartsys::core::settings;
    using iotsmartsys::core::common::StateResult;

    // Parse mqtt sub-object at path base (e.g. "mqtt.primary")
    static StateResult parseMqttConfig(const iotsmartsys::platform::common::json::JsonPathExtractor &ext, const char *base, MqttConfig &out, bool allowTtl)
    {
        // defaults
        out = MqttConfig{};
        out.port = 1883;
        out.protocol = "mqtt";
        out.ttl = 0;

        // host obrigatório
        std::string host;
        std::string pathHost = std::string(base) + ".host";
        if (!ext.getString(pathHost.c_str(), host) || host.empty())
            return StateResult::InvalidState;
        out.host = host;

        // port (optional, default already set)
        int port = 1883;
        std::string pathPort = std::string(base) + ".port";
        if (ext.getInt(pathPort.c_str(), port))
        {
            if (port <= 0 || port > 65535)
                return StateResult::Overflow;
            out.port = port;
        }

        // user/password optional
        std::string tmp;
        if (ext.getString((std::string(base) + ".user").c_str(), tmp)) out.user = tmp;
        if (ext.getString((std::string(base) + ".password").c_str(), tmp)) out.password = tmp;

        // protocol optional
        if (ext.getString((std::string(base) + ".protocol").c_str(), tmp)) out.protocol = tmp;
        if (out.protocol.empty()) out.protocol = "mqtt";

        if (allowTtl)
        {
            int ttl = 0;
            if (ext.getInt((std::string(base) + ".ttl").c_str(), ttl))
            {
                if (ttl < 0 || ttl > 1440)
                    return StateResult::Overflow;
                out.ttl = ttl;
            }
        }

        if (!out.isValid())
            return StateResult::InvalidState;

        return StateResult::Ok;
    }

    static StateResult parseMqtt(const iotsmartsys::platform::common::json::JsonPathExtractor &ext, MqttSettings &out)
    {
        out.profile = "primary";

        // primary required
        StateResult err = parseMqttConfig(ext, "mqtt.primary", out.primary, /*allowTtl*/ false);
        if (err != StateResult::Ok) return err;

        // secondary optional
        // detect presence by trying to get host
        std::string tmp;
        if (ext.getString("mqtt.secondary.host", tmp))
        {
            err = parseMqttConfig(ext, "mqtt.secondary", out.secondary, /*allowTtl*/ true);
            if (err != StateResult::Ok) return err;
        }
        else
        {
            out.secondary = MqttConfig{};
        }

        // topics optional
        if (ext.getString("mqtt.topic.announce", tmp)) out.announce_topic = tmp;
        if (ext.getString("mqtt.topic.command", tmp)) out.command_topic = tmp;
        if (ext.getString("mqtt.topic.notify", tmp)) out.notify_topic = tmp;
        if (ext.getString("mqtt.profile", tmp) && !tmp.empty()) out.profile = tmp;

        return StateResult::Ok;
    }

    static StateResult parseFirmware(const iotsmartsys::platform::common::json::JsonPathExtractor &ext, FirmwareConfig &out)
    {
        std::string update;
        if (!ext.getString("firmware.update", update))
            update = "none";

        std::string url, manifest;
        bool verify = false;

        if (update == "auto")
        {
            if (!ext.getString("firmware.url", url) || url.empty()) return StateResult::InvalidState;
            if (!ext.getString("firmware.manifest", manifest) || manifest.empty()) return StateResult::InvalidState;
        }

        if (ext.getBool("firmware.verifysha256", verify)) {}

        out.url = url;
        out.manifest = manifest;
        out.verify_sha256 = verify;
        out.update = update;
        return StateResult::Ok;
    }

    static StateResult parseWifi(const iotsmartsys::platform::common::json::JsonPathExtractor &ext, WifiConfig &out)
    {
        std::string tmp;
        (void)ext.getString("wifi.ssid", out.ssid);
        (void)ext.getString("wifi.password", out.password);
        return StateResult::Ok;
    }

    static StateResult parseApi(const iotsmartsys::platform::common::json::JsonPathExtractor &ext, ApiConfig &out)
    {
        (void)ext.getString("api.url", out.url);
        (void)ext.getString("api.key", out.key);
        (void)ext.getString("api.basic_auth", out.basic_auth);
        return StateResult::Ok;
    }

    iotsmartsys::core::common::StateResult EspIdfSettingsParser::parse(const char *json, Settings &out)
    {
        if (!json || *json == '\0')
            return StateResult::InvalidArg;

        size_t len = strlen(json);
        iotsmartsys::platform::common::json::JsonPathExtractor ext(json, len);

        StateResult result = StateResult::Ok;

        // mqtt obrigatório
        result = parseMqtt(ext, out.mqtt);
        if (result != StateResult::Ok) return result;

        // firmware obrigatório
        result = parseFirmware(ext, out.firmware);
        if (result != StateResult::Ok) return result;

        // wifi optional
        (void)parseWifi(ext, out.wifi);

        // api optional
        (void)parseApi(ext, out.api);

        // in_config_mode optional
        bool icm = false;
        if (ext.getBool("in_config_mode", icm)) out.in_config_mode = icm;

        // prefix_auto_format_properies_json -- ignored (kept parity with previous behaviour)

        // log_level optional
        std::string logLevel;
        if (ext.getString("log_level", logLevel))
        {
            if (logLevel == "trace") out.logLevel = iotsmartsys::core::LogLevel::Trace;
            else if (logLevel == "debug") out.logLevel = iotsmartsys::core::LogLevel::Debug;
            else if (logLevel == "info") out.logLevel = iotsmartsys::core::LogLevel::Info;
            else if (logLevel == "warn") out.logLevel = iotsmartsys::core::LogLevel::Warn;
            else if (logLevel == "error") out.logLevel = iotsmartsys::core::LogLevel::Error;
        }

        return StateResult::Ok;
    }
} // namespace iotsmartsys::platform::espressif
