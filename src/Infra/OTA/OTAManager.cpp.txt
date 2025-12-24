#include "OTAManager.h"
#include "Infra/Settings/ConfigManager.h"
#include "Infra/OTA/OTA.h"
#include "Infra/OTA/FirmwareUpdater.h"
#include "Infra/Utils/Logger.h"

using namespace OTA;

void OTA::setup(FirmwareConfig settings)
{
    LOG_PRINTLN("[OTA] Inicializando OTA a partir das configurações...");

    switch (settings.update)
    {
    case FirmwareUpdateMethod::NONE:
        LOG_PRINTLN("[OTA] Atualizações desabilitadas (FirmwareUpdateMethod::NONE).");
        return;
    case FirmwareUpdateMethod::OTA:
        LOG_PRINTLN("[OTA] Método de atualização definido como OTA.");
        setupOTA();
        break;
    case FirmwareUpdateMethod::AUTO:
        LOG_PRINTLN("[OTA] Método de atualização definido como AUTO.");
        update(settings);
        break;
    default:
        LOG_PRINTLN("[OTA] Método de atualização desconhecido. Abortando configuração de OTA.");
        return;
    }
}

void OTA::update(FirmwareConfig settings)
{
    String baseUrl = settings.url;
    String manifestUrl = baseUrl + settings.manifest;

    bool verifySha = settings.verify_sha256;

    // configure global updater
    fwUpdater.configure(manifestUrl, baseUrl, verifySha);

    fwUpdater.checkAndUpdate();
}
