#pragma once

#include <Arduino.h>
#include "Models/Settings.h"

class SettingsFetcher
{
public:
  SettingsFetcher() = default;

  // Busca as configurações do endpoint remoto. Retorna true em caso de sucesso
  // e preenche `out` com os valores.
  // deviceId: id do device a ser colocado na URL
  // As credenciais e a URL base são lidas das build flags (macros):
  //  - SETTINGS_API_BASE_URL
  //  - SETTINGS_API_KEY
  //  - SETTINGS_AUTH_BASIC
  // timeoutMs: tempo máximo em ms para a requisição
  bool fetchSettings(const String &deviceId,
                     Settings &out,
                     unsigned long timeoutMs = 10000UL) const;


  private:
  String buildUrl(const String &deviceId) const;
  bool parseSettingsJson(const String &payload, Settings &settings) const;
};