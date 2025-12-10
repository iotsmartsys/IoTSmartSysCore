#pragma once

#include <Arduino.h>
#include "Infra/Settings/Models/Settings.h"
namespace OTA
{
    class FirmwareUpdater
    {
    public:
    FirmwareUpdater();
    FirmwareUpdater(const String &manifestUrl);

    // Configure runtime parameters (manifest URL, base URL, TLS and checksum behavior,
    // and optional module/env used to validate the manifest).
    void configure(const String &manifestUrl,
               const String &baseUrl = String(),
               bool verifySha256 = false);

    void checkAndUpdate();

    private:
        String _manifestUrl;
        String _baseUrl;
        bool _useHttps;
        bool _verifySha256;

        struct ManifestInfo
        {
            String module;
            String env;
            String version;
            String urlPath; // path do .bin
            String fullUrl; // URL final completa
            String checksumType;
            String checksumValue;
            size_t size = 0;
            bool mandatory = false;
            bool valid = false;
        };

        ManifestInfo fetchManifest();
        bool isRemoteNewer(const String &remoteVersion);
        bool performOta(const ManifestInfo &manifest);
    };

}

// Se houver uma instância global definida em um arquivo de implementação (por exemplo, `src/main.cpp`)
// nós expomos aqui a declaração extern para que outras unidades de compilação possam referenciá-la.
// A definição real deve existir em apenas um .cpp (ex: `FirmwareUpdater fwUpdater(...);`).
extern OTA::FirmwareUpdater fwUpdater;