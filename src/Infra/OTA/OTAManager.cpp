#include "OTAManager.h"
#include "Infra/OTA/OTA.h"

using namespace iotsmartsys::ota;

OTAManager::OTAManager(IReadOnlySettingsProvider &settingsProvider, ILogger &logger, IFirmwareManifestParser &manifestParser, OTA &ota)
    : _settingsProvider(settingsProvider),
      _firmwareUpdater(logger, manifestParser),
      _logger(logger),
      _ota(ota)
{
}

void OTAManager::handle()
{
    if (!_settingsProvider.hasCurrent())
    {
        _logger.warn("[OTA]", "Nenhuma configuração atual disponível. Abortando OTA.");
        return;
    }

    _logger.info("[OTA]", "Inicializando OTA a partir das configurações...");
    settings::FirmwareConfig firmwareSettings;
    settings::Settings currentSettings;
    if (_settingsProvider.copyCurrent(currentSettings))
    {
        firmwareSettings = currentSettings.firmware;
    }
    else
    {
        _logger.error("[OTA]", "Falha ao obter configurações atuais. Abortando OTA.");
        return;
    }

    switch (firmwareSettings.update)
    {
    case FirmwareUpdateMethod::NONE:
        _logger.info("[OTA]", "Atualizações desabilitadas (FirmwareUpdateMethod::NONE).");
        return;
    case FirmwareUpdateMethod::OTA:
        if (!_ota.isInitialized())
        {
            _logger.info("[OTA]", "Método de atualização definido como OTA.");
            _ota.setup();
        }

        _ota.handle();
        break;
    case FirmwareUpdateMethod::AUTO:
        _logger.info("[OTA]", "Método de atualização definido como AUTO.");
        update(firmwareSettings);
        break;
    default:
        _logger.error("[OTA]", "Método de atualização desconhecido. Abortando configuração de OTA.");
        return;
    }
}

void OTAManager::update(FirmwareConfig firmwareSettings)
{
    if (_firmwareUpdater.hasCheckedForUpdate())
    {
        _logger.info("[OTA]", "Verificação de atualização já realizada nesta sessão. Pulando.");
        return;
    }
    
    std::string baseUrl = firmwareSettings.url;
    std::string manifestUrl = baseUrl + firmwareSettings.manifest;

    bool verifySha = firmwareSettings.verify_sha256;

    _firmwareUpdater.checkAndUpdate(firmwareSettings);
}
