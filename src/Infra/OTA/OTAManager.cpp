#include "OTAManager.h"
#include "Contracts/Connectivity/ConnectivityGate.h"

using namespace iotsmartsys::ota;

OTAManager::OTAManager(IReadOnlySettingsProvider &settingsProvider, ILogger &logger, IFirmwareManifestParser &manifestParser,
#ifdef OTA_ENABLED
                       OTA &ota,
#endif
                       iotsmartsys::core::settings::ISettingsGate &settingsGate)
    : _settingsProvider(settingsProvider),
      _firmwareUpdater(logger, manifestParser),
      _logger(logger),
#ifdef OTA_ENABLED
      _ota(ota),
#endif
      _settingsGate(settingsGate)
{
}

void OTAManager::handle()
{
    auto &gate = iotsmartsys::core::ConnectivityGate::instance();
    const bool networkReady = gate.isNetworkReady();
    const bool settingsReady = _settingsGate.level() >= iotsmartsys::core::settings::SettingsReadyLevel::Available;

    if (settingsReady != _lastSettingsReady)
    {
        if (settingsReady)
        {
            _logger.info("[OTA Manager]", "SettingsReady=TRUE (cache OK). OTA liberado quando NetworkReady=TRUE.");
        }
        else
        {
            _logger.warn("[OTA Manager]", "SettingsReady=FALSE (cache ainda não carregou). Bloqueando OTA.");
        }
        _lastSettingsReady = settingsReady;
    }

    if (networkReady != _lastNetworkReady)
    {
        if (networkReady)
        {
            _logger.info("[OTA Manager]", "NetworkReady=TRUE (Wi-Fi/IP). OTA liberado quando SettingsReady=TRUE.");
        }
        else
        {
            _logger.warn("[OTA Manager]", "NetworkReady=FALSE (Wi-Fi/IP não pronto). Bloqueando OTA.");
        }
        _lastNetworkReady = networkReady;
    }

    if (!settingsReady || !networkReady)
        return;

    if (!_settingsProvider.hasCurrent())
    {
        _logger.warn("[OTA Manager]", "Nenhuma configuração atual disponível. Abortando OTA.");
        return;
    }

    _logger.debug("[OTA Manager]", "Inicializando OTA a partir das configurações...");
    settings::FirmwareConfig firmwareSettings;
    settings::Settings currentSettings;
    if (_settingsProvider.copyCurrent(currentSettings))
    {
        _logger.debug("[OTA Manager]", "Configurações atuais obtidas com sucesso.");
        _logger.debug("[OTA Manager]", "Firmware URL: %s", currentSettings.firmware.url.c_str());
        _logger.debug("[OTA Manager]", "Firmware Manifest: %s", currentSettings.firmware.manifest.c_str());
        _logger.debug("[OTA Manager]", "Firmware Update Method: %d", (int)currentSettings.firmware.update);

        firmwareSettings = currentSettings.firmware;
    }
    else
    {
        _logger.error("[OTA Manager]", "Falha ao obter configurações atuais. Abortando OTA.");
        return;
    }

    switch (firmwareSettings.update)
    {
    case FirmwareUpdateMethod::NONE:
        _logger.debug("[OTA Manager]", "Atualizações desabilitadas (FirmwareUpdateMethod::NONE).");
        return;
    case FirmwareUpdateMethod::OTA:
#ifdef OTA_ENABLED
        if (!_ota.isInitialized())
        {
            _logger.debug("[OTA Manager]", "Método de atualização definido como OTA.");
            _ota.setup();
        }

        _ota.handle();
#endif
        break;
    case FirmwareUpdateMethod::AUTO:
        _logger.debug("[OTA Manager]", "Método de atualização definido como AUTO.");
        update(firmwareSettings);
        break;
    default:
        _logger.error("[OTA Manager]", "Método de atualização desconhecido. Abortando configuração de OTA.");
        return;
    }
}

void OTAManager::update(FirmwareConfig firmwareSettings)
{
    if (_firmwareUpdater.hasCheckedForUpdate())
    {
        _logger.debug("[OTA Manager]", "Verificação de atualização já realizada nesta sessão. Pulando.");
        return;
    }

    std::string baseUrl = firmwareSettings.url;
    std::string manifestUrl = baseUrl + firmwareSettings.manifest;

    bool verifySha = firmwareSettings.verify_sha256;

    _firmwareUpdater.checkAndUpdate(firmwareSettings);
}
