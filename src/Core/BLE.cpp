#include "BLE.h"
#include <Arduino.h>
#include "Utils/Logger.h"

#if defined(NODE_BLE_ENABLED)
#include <NimBLEDevice.h>
#include "Managers/RemoteCapabilitiesFetcher.h"
#include "Transports/IMessageClient.h"
#include "Capabilities/CapabilityCommand.h"

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_CMD_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_RESP_UUID "12345678-1234-5678-1234-56789abcdef0"


NimBLECharacteristic *commandChar = nullptr;
NimBLECharacteristic *responseChar = nullptr;
bool _isBLEConnected = false;


unsigned long lastSendTime = 0;
int currentCapIndex = 0;
std::vector<CapabilityTiny> capabilitiesToSend;

String extractJsonField(const String& json, const String& field) {
    int idx = json.indexOf(field);
    if (idx < 0) return "";
    int start = json.indexOf(':', idx) + 1;
    int asp1 = json.indexOf('\"', start) + 1;
    int asp2 = json.indexOf('\"', asp1);
    return (asp1 >= 0 && asp2 > asp1) ? json.substring(asp1, asp2) : "";
}

void sendNextCapability() {
    if (!_isBLEConnected || !responseChar || currentCapIndex >= capabilitiesToSend.size())
        return;

    unsigned long now = millis();
    if (lastSendTime == 0 || now - lastSendTime >= 100) {
        CapabilityTiny &cap = capabilitiesToSend[currentCapIndex];
        String cmdStr = String("{\"capability_name\":\"") + cap.capability_name +
                        "\",\"value\":\"" + cap.value +
                        "\",\"updated_at\":\"" + cap.updated_at + "\"}";
        LOG_PRINT("🔵 Enviando comando via BLE: ");
        LOG_PRINTLN(cmdStr);

        responseChar->setValue(cmdStr);
        responseChar->notify();
        lastSendTime = now;
        currentCapIndex++;
        if (currentCapIndex >= capabilitiesToSend.size()) {
            capabilitiesToSend.clear();
        }
    }
}


class MyServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer *pServer) override {
        _isBLEConnected = true;
        LOG_PRINTLN("✅ Cliente BLE conectado");
    }
    void onDisconnect(NimBLEServer *pServer) override {
        LOG_PRINTLN("⚠️ Cliente BLE desconectado, reiniciando advertising");
        _isBLEConnected = false;
        pServer->getAdvertising()->start();
    }
};


void setupBLEGateway() {
    LOG_PRINTLN("🔵 Iniciando BLE Gateway...");
    esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);

    NimBLEDevice::init("ESP32_Commander");
    NimBLEServer *pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    NimBLEService *pService = pServer->createService(SERVICE_UUID);

    commandChar = pService->createCharacteristic(
        CHARACTERISTIC_CMD_UUID,
        NIMBLE_PROPERTY::WRITE);

    responseChar = pService->createCharacteristic(
        CHARACTERISTIC_RESP_UUID,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    pService->start();

    NimBLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    LOG_PRINTLN("📡 BLE advertising iniciado");
}

void notifyBLEGateway(byte *payload) {
    if (!responseChar || !_isBLEConnected) return;

    CapabilityCommand command;
    if (command.fromJson((char *)payload)) {
        if (command.capability_name == "TIME_SCHEDULE") return;
        String cmdStr = String("{\"capability_name\":\"") + command.capability_name +
                        "\",\"value\":\"" + command.value +
                        "\",\"updated_at\":\"now\"}";
        LOG_PRINT("🔵 Enviando comando via BLE: ");
        LOG_PRINTLN(cmdStr);

        responseChar->setValue(cmdStr);
        responseChar->notify();
    }
}

void handleBLEGateway(IMessageClient *transport) {
    std::string cmd = commandChar->getValue();
    if (!cmd.empty() && cmd.length() > 10) {
        LOG_PRINT("🔵 Comando recebido via BLE: ");
        LOG_PRINTLN(cmd.c_str());

        String cmdStr(cmd.c_str());
        String capability_name = extractJsonField(cmdStr, "\"capability_name\"");
        String value = extractJsonField(cmdStr, "\"value\"");
        String device_id = extractJsonField(cmdStr, "\"device_id\"");

        LOG_PRINTLN("📨 Payload recebido:");
        LOG_PRINT(" - capability_name: "); LOG_PRINTLN(capability_name);
        LOG_PRINT(" - value: "); LOG_PRINTLN(value);
        LOG_PRINT(" - device_id: "); LOG_PRINTLN(device_id);

        if (transport) {
            String topicStr = String("device/") + device_id + "/command";
            LOG_PRINTLN("🔗 Enviando comando para MQTT...");
            transport->sendMessage(topicStr.c_str(), cmdStr.c_str());
        }
        commandChar->setValue(""); 
        LOG_PRINTLN("✅ Comando processado e resposta enviada via BLE.");
    }
    sendNextCapability();
}

bool isBLEConnected() {
    return _isBLEConnected;
}

#endif
