#include "MqttClientHandler.h"
#ifdef ESP32

#else
#include <ESP8266HTTPClient.h>
#endif
#include "Core/CommandExecutor.h"
#include "Settings/VersionInfo.h"
#include "Wifi/WifiHelper.h"
#include "Transports/BridgeHooks.h"
#include "Utils/Logger.h"

#include "Mqtt/AnnouncePayloadBuilder.h"

#ifdef ST7789_170x320_ENABLED
#include "display/Display_ST7789_170_320.h"
#endif

#if defined(NODE_BLE_ENABLED)
#include "Core/BLE.h"
#endif

MqttClientHandler::MqttClientHandler(const char *device_name, MqttSettings mqttSettings, const std::vector<Capability *> &capabilities, const std::vector<Property *> &properties, AnnouncePayloadBuilder *announceBuilder)
    : mqttSettings(mqttSettings),
      capabilities(capabilities),
      properties(properties),
      device_name(device_name),
      announceBuilder(announceBuilder)
{
    device_id = getDeviceId();
    mac_address = getMacAddress();
    currentMqttConfig = mqttSettings.primary;
}

void MqttClientHandler::announceDevice()
{
    String payload;
    if (announceBuilder != nullptr)
    {
        payload = announceBuilder->buildDevicePayload();
    }
    else
    {
        // fallback: criar um builder temporário local com os dados atuais
        AnnouncePayloadBuilder tmp(capabilities, properties);
        payload = tmp.buildDevicePayload();
    }

    LOG_PRINTLN("Anunciando o dispositivo...");
    sendMqttMessage(mqttSettings.announce_topic.c_str(), payload);
    LOG_PRINTLN("Dispositivo anunciado com sucesso!");
}

void MqttClientHandler::sendMqttMessage(const char *topic, String payload)
{
    LOG_PRINT("Publicando no tópico: ");
    LOG_PRINTLN(topic);
    LOG_PRINT("Payload: ");
    LOG_PRINTLN(payload);
    if (client.publish(topic, payload.c_str()))
    {
        LOG_PRINTLN("Publicado com sucesso!");
    }
    else
    {
        LOG_PRINTLN("Falha ao publicar! Verifique o tamnho do payload e as configurações do broker MQTT.");
    }
}

bool MqttClientHandler::subscribeDeviceCommand(const String &devId)
{
    if (!client.connected())
    {
        LOG_PRINTLN("[MQTT] subscribeDeviceCommand: cliente MQTT desconectado");
        return false;
    }
    String topic = String("device/") + devId + "/command";
    LOG_PRINT("Assinando o tópico: ");
    LOG_PRINTLN(topic);
    bool ok = client.subscribe(topic.c_str());
    LOG_PRINTLN(ok ? "Assinatura realizada com sucesso!" : "Falha ao assinar o tópico!");
    return ok;
}

void MqttClientHandler::subscribe(const char *topic)
{
    if (!client.connected())
    {
        LOG_PRINTLN("[MQTT] subscribe: cliente MQTT desconectado");
        return;
    }
    LOG_PRINT("Assinando o tópico: ");
    LOG_PRINTLN(topic);
    if (client.subscribe(topic))
    {
        LOG_PRINTLN("Assinatura realizada com sucesso!");
    }
    else
    {
        LOG_PRINTLN("Falha ao assinar o tópico!");
    }
}

void MqttClientHandler::callbackMqtt(char *topic, byte *payload, unsigned int length)
{
    LOG_PRINT("Mensagem recebida no tópico: ");
    LOG_PRINTLN(topic);
    LOG_PRINT("Payload: ");
    LOG_PRINTF("%.*s", length, (const char *)payload);
    LOG_PRINTLN("");

    String cmdStr;
    cmdStr.reserve(length + 1);
    for (unsigned int i = 0; i < length; ++i)
    {
        cmdStr += static_cast<char>(payload[i]);
    }

    CapabilityCommand command;
    if (command.fromJson(cmdStr.c_str()))
    {

#ifdef ST7789_170x320_ENABLED
        displayInfo("Comando MQTT: " + cmdStr);
#endif

        if (command.device_id != device_id)
        {
            LOG_PRINTLN("Comando destinado a outro dispositivo. Ignorando...");
            return;
        }
        extern void applyCommandToCapabilities(const CapabilityCommand &command,
                                               std::vector<Capability *> &capabilities,
                                               bool &power_on,
                                               MqttClientHandler *mqttHandler);
        applyCommandToCapabilities(command, const_cast<std::vector<Capability *> &>(capabilities), power_on, this);

        mqtt_to_espnow_forward(command);
    }
    else
    {
        LOG_PRINTLN("Erro ao desserializar a mensagem.");
    }
}

void MqttClientHandler::setup()
{
    selectBroker(BrokerType::PRIMARY);
    client.setBufferSize(1024);
    client.setKeepAlive(60);
    client.setCallback([this](char *topic, byte *payload, unsigned int length)
                       { this->callbackMqtt(topic, payload, length); });
}

void MqttClientHandler::handle()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        LOG_PRINTLN("Wi-Fi ainda não está conectado. Aguardando...");
        return;
    }

    if (!client.connected())
    {
        unsigned long now = millis();
        if (!reconnecting || (now - lastReconnectAttempt > reconnectIntervalMs))
        {
            reconnecting = true;
            lastReconnectAttempt = now;
            LOG_PRINTLN("Tentando reconectar ao MQTT...");
            client.disconnect();

            delay(100);

            selectBroker(BrokerType::PRIMARY);
            if (connectCurrent())
            {
                LOG_PRINTLN("Conectado com sucesso!");

                subscribeToCommandTopic();
                reconnecting = false;
            }
            else
            {
                LOG_PRINT("Falha ao conectar: ");
                LOG_PRINTLN(client.state());

                if (mqttSettings.hasSecondary())
                {
                    LOG_PRINTLN("Tentando broker secundário (fallback)...");
                    selectBroker(BrokerType::SECONDARY);
                    delay(100);
                    if (connectCurrent())
                    {
                        LOG_PRINTLN("Conectado ao broker secundário.");
                        if (getCurrentMqttConfig().ttl == 0)
                        {
                            LOG_PRINTLN("Atenção: TTL do broker secundário está definido como 0. O fallback não será verificado novamente.");
                            subscribeToCommandTopic();
                            reconnecting = false;
                        }
                        else
                        {
                            LOG_PRINTLN("Conectado ao broker secundário. Tentaremos reconectar ao primário após o TTL (" + String(getCurrentMqttConfig().ttl) + " minutos).");
                        }

                        subscribeToCommandTopic();
                        reconnecting = false;
                    }
                    else
                    {
                        LOG_PRINT("Falha também no secundário: ");
                        LOG_PRINTLN(client.state());
                    }
                }
            }
        }
    }
    else
    {
        client.loop();

        if (currentBroker == BrokerType::SECONDARY)
        {
            unsigned long probeInterval = currentMqttConfig.getReconnectIntervalMs();
            if (probeInterval > 0 && (millis() - lastPrimaryProbe > probeInterval))
            {
                lastPrimaryProbe = millis();
                LOG_PRINTLN("Verificando disponibilidade do broker primário...");

                client.disconnect();
                delay(100);
                selectBroker(BrokerType::PRIMARY);
                if (connectCurrent())
                {
                    LOG_PRINTLN("Migrado de volta ao broker primário.");
                    subscribeToCommandTopic();
                }
                else
                {
                    LOG_PRINTLN("Primário indisponível. Retornando ao secundário.");
                    selectBroker(BrokerType::SECONDARY);
                    if (connectCurrent())
                    {
                        subscribeToCommandTopic();
                    }
                }
            }
        }
    }
}

void MqttClientHandler::sendState(CapabilityState state)
{
    state.device_id = device_id;
    String payload = state.toJson();
    String topic = mqttSettings.notify_topic;
    LOG_PRINT("Publicando no tópico: ");
    LOG_PRINTLN(topic);
    LOG_PRINT("Payload: ");
    LOG_PRINTLN(payload);
    if (client.publish(topic.c_str(), payload.c_str()))
    {
        LOG_PRINTLN("Publicado com sucesso!");
    }
    else
    {
        LOG_PRINTLN("Falha ao publicar! Verifique o tamnho do payload e as configurações do broker MQTT.");
    }
}
bool MqttClientHandler::isPowerOn()
{
    return power_on;
}

bool MqttClientHandler::isConnected()
{
    return client.connected();
}

Client &MqttClientHandler::getCurrentClient(const MqttConfig &mqttConfig)
{
    // Se o protocolo for MQTT sobre TLS, usamos o cliente seguro.
    if (mqttConfig.protocol == "mqtts")
    {
        return wifiSecureClient;
    }

    // Caso contrário, usamos o cliente TCP "puro".
    return wifiClient;
}

// MqttConfig MqttClientHandler::getCurrentMqttConfig()
// {
//     return (currentBroker == BrokerType::SECONDARY && mqttSettings.hasSecondary())
//                ? mqttSettings.secondary
//                : mqttSettings.primary;
// }

void MqttClientHandler::selectBroker(BrokerType target)
{
    currentBroker = target;

    if (target == BrokerType::SECONDARY && !mqttSettings.hasSecondary())
    {
        currentBroker = BrokerType::PRIMARY;
    }

    currentMqttConfig = (currentBroker == BrokerType::SECONDARY)
                            ? mqttSettings.secondary
                            : mqttSettings.primary;

    if (currentBroker == BrokerType::SECONDARY)
    {
        lastPrimaryProbe = millis(); // reinicia o timer antes de tentar voltar ao primário
    }

    // Se for MQTT sobre TLS, configuramos o cliente seguro.
    if (currentMqttConfig.protocol == "mqtts")
    {
        // TODO: futuramente podemos trocar setInsecure() por configuração de CA/certificados.
        wifiSecureClient.setInsecure();
    }

    // Escolhe o cliente adequado (TCP ou TLS) baseado no protocolo.
    Client &mqttClientTransport = getCurrentClient(currentMqttConfig);
    client.setClient(mqttClientTransport);

    client.setServer(currentMqttConfig.host.c_str(), currentMqttConfig.port);
}

void MqttClientHandler::updatePropertiesInfo(std::vector<Property *> &properties)
{
    for (const auto &prop : properties)
    {
        if (prop->property_name == "wifi_ssid")
        {
            prop->value = String(WiFi.SSID());
        }
        else if (prop->property_name == "wifi_signal")
        {
            prop->value = String(WiFi.RSSI());
        }
        else if (prop->property_name == "broker")
        {
            prop->value = String(getCurrentMqttConfig().host);
        }
    }
}

MqttConfig MqttClientHandler::getCurrentMqttConfig() const
{
    return currentMqttConfig;
}

bool MqttClientHandler::connectCurrent()
{
    MqttConfig mqttConfig = getCurrentMqttConfig();

    LOG_PRINT("Conectando ao servidor MQTT: ");
    LOG_PRINT(mqttConfig.host);
    LOG_PRINT(":");
    LOG_PRINTLN(mqttConfig.port);
    const char *mqtt_user = mqttConfig.user.c_str();
    const char *mqtt_password = mqttConfig.password.c_str();

    bool connected = client.connect(device_id.c_str(), mqtt_user, mqtt_password);
    LOG_PRINTLN("Tentativa de conexão com o broker MQTT...");
    if (connected)
    {
        updatePropertiesInfo(properties);
        LOG_PRINTLN("Conectado com sucesso!");
    }
    else
    {
        LOG_PRINT("Falha ao conectar: ");
        LOG_PRINTLN(client.state());
    }
    return connected;
}

bool MqttClientHandler::subscribeToCommandTopic()
{
    String device_command_topic = mqttSettings.getCommandTopicForDevice(device_id);
    LOG_PRINT("Assinando o tópico: ");
    LOG_PRINTLN(device_command_topic);
    bool subscribed = client.subscribe(device_command_topic.c_str());
    LOG_PRINTLN(subscribed ? "Assinatura realizada com sucesso!" : "Falha ao assinar o tópico!");
    if (subscribed)
    {
        announceDevice();
    }
    return subscribed;
}
