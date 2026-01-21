#include "Arduino.h"
#include "FirmwareUpdater.h"
#if defined(ESP32)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Update.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Updater.h>
#include <BearSSLHelpers.h>
#endif
#include "Version/VersionInfo.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include "Sha256Context.h"

namespace iotsmartsys::ota
{

    static const int FW_OTA_MAX_HTTP_RETRIES = 3;

    static bool isHttpsUrl(const std::string &url)
    {
        return url.rfind("https://", 0) == 0;
    }

    FirmwareUpdater::FirmwareUpdater(ILogger &logger, IFirmwareManifestParser &manifestParser)
        : _logger(logger),
          _manifestUrl(""),
          _baseUrl(""),
          _useHttps(false),
          _verifySha256(false),
          _manifestParser(manifestParser)
    {
    }

    ManifestInfo FirmwareUpdater::fetchManifest()
    {
        ManifestInfo info;

        _logger.debug("FW-OTA", "fetchManifest(): _manifestUrl='%s' _baseUrl='%s' useHttps=%d verifySha256=%d",
                      _manifestUrl.c_str(),
                      _baseUrl.c_str(),
                      _useHttps ? 1 : 0,
                      _verifySha256 ? 1 : 0);

        if (WiFi.status() != WL_CONNECTED)
        {
            _logger.warn("[FW-OTA] WiFi não conectado, pulando fetch do manifest.");
            return info;
        }

        if (_manifestUrl.length() == 0)
        {
            _logger.warn("[FW-OTA] Manifest URL vazia. Abortando fetch do manifest.");
            return info;
        }

        // Se a URL do manifest for relativa, prefixa com baseUrl
        std::string manifestUrl = _manifestUrl;
        const bool manifestIsAbsolute = isHttpsUrl(manifestUrl) || manifestUrl.rfind("http://", 0) == 0;
        if (!manifestIsAbsolute)
        {
            if (_baseUrl.empty())
            {
                _logger.error("FW-OTA", "Manifest URL é relativa e Base URL está vazia. Abortando.");
                return info;
            }

            const bool baseHasSlash = !_baseUrl.empty() && _baseUrl.back() == '/';
            const bool manifestHasSlash = !manifestUrl.empty() && manifestUrl.front() == '/';

            if (baseHasSlash && manifestHasSlash)
            {
                manifestUrl = _baseUrl + manifestUrl.substr(1);
            }
            else if (!baseHasSlash && !manifestHasSlash)
            {
                manifestUrl = _baseUrl + "/" + manifestUrl;
            }
            else
            {
                manifestUrl = _baseUrl + manifestUrl;
            }
        }

        _logger.info("FW-OTA", "Baixando manifest em: %s", manifestUrl.c_str());

        std::string payload;

        for (int attempt = 1; attempt <= FW_OTA_MAX_HTTP_RETRIES; ++attempt)
        {
            HTTPClient http;
            http.setTimeout(10000); // 10 segundos

            WiFiClientSecure httpsClient;
            WiFiClient httpClient;

            bool useHttps = isHttpsUrl(manifestUrl) || _useHttps;

            if (useHttps)
            {
                _logger.info("FW-OTA", "Usando HTTPS para baixar o manifest.");
                httpsClient.setInsecure();

                if (!http.begin(httpsClient, String(manifestUrl.c_str())))
                {
                    _logger.error("FW-OTA", "Falha em iniciar HTTPClient (HTTPS).");
                    http.end();
                }
                else
                {
                    int httpCode = http.GET();
                    if (httpCode == HTTP_CODE_OK)
                    {
                        payload = http.getString().c_str();
                        http.end();
                        break;
                    }

                    _logger.error("FW-OTA", "Erro ao baixar manifest (HTTPS). HTTP code: %d", httpCode);
                    _logger.error("FW-OTA", " (%s)", http.errorToString(httpCode).c_str());
                    http.end();
                }
            }
            else
            {
                _logger.info("FW-OTA", "Usando HTTP para baixar o manifest.");

                if (!http.begin(httpClient, String(manifestUrl.c_str())))
                {
                    _logger.error("FW-OTA", "Falha em iniciar HTTPClient (HTTP).");
                    http.end();
                }
                else
                {
                    int httpCode = http.GET();
                    if (httpCode == HTTP_CODE_OK)
                    {
                        payload = http.getString().c_str();
                        http.end();
                        break;
                    }

                    _logger.error("FW-OTA", "Erro ao baixar manifest (HTTP). HTTP code: %d", httpCode);
                    _logger.error("FW-OTA", " (%s)", http.errorToString(httpCode).c_str());
                    http.end();
                }
            }

            if (attempt < FW_OTA_MAX_HTTP_RETRIES)
            {
                _logger.info("FW-OTA", "Tentando novamente fetch do manifest (tentativa %d)...", attempt + 1);
                delay(500);
            }
        }

        if (!payload.length())
        {
            _logger.error("FW-OTA", "Falha ao obter manifest após múltiplas tentativas.");
            return info;
        }

        _logger.info("FW-OTA", "Manifest recebido (%u bytes):", (unsigned)payload.size());
        _logger.info("FW-OTA", "%s", payload.c_str());

        _manifestParser.parseManifest(payload.c_str(), payload.length(), info);

        if (info.version.length() == 0 || info.urlPath.length() == 0)
        {
            _logger.warn("FW-OTA", "Manifest inválido: sem version ou urlPath.");
            return info;
        }

        // Monta URL completa:
        if (info.urlPath.rfind("http://", 0) == 0 || info.urlPath.rfind("https://", 0) == 0)
        {
            info.fullUrl = info.urlPath;
        }
        else
        {
            std::string base = _baseUrl;
            info.fullUrl = base + info.urlPath;
        }

        info.valid = true;
        return info;
    }

    void FirmwareUpdater::configure(const std::string &manifestUrl,
                                    const std::string &baseUrl,
                                    bool verifySha256)
    {
        _logger.info("FW-OTA", "Configurando FirmwareUpdater...");
        _logger.debug("FW-OTA", "configure(): manifestUrl(in)='%s' baseUrl(in)='%s' verifySha256=%d",
                      manifestUrl.c_str(),
                      baseUrl.c_str(),
                      verifySha256 ? 1 : 0);

        if (manifestUrl.length())
            _manifestUrl = manifestUrl;
        _logger.info("FW-OTA", "Manifest URL: %s", _manifestUrl.c_str());
        if (baseUrl.length())
            _baseUrl = baseUrl;
        _logger.info("FW-OTA", "Base URL: %s", _baseUrl.c_str());

        // Decide HTTPS com base tanto na baseUrl quanto na manifestUrl.
        _useHttps = isHttpsUrl(_baseUrl) || isHttpsUrl(_manifestUrl);

        _logger.info("FW-OTA", "Usando HTTPS: %s", _useHttps ? "true" : "false");
        _verifySha256 = verifySha256;
    }

    bool FirmwareUpdater::isRemoteNewer(const ManifestInfo &manifest)
    {
        std::string currentVersion = getBuildIdentifier();

        _logger.debug("FW-OTA", "Comparando versões...");
        _logger.debug("FW-OTA", "Versão atual: %s", currentVersion.c_str());
        _logger.debug("FW-OTA", "Versão remota: %s", manifest.version.c_str());

        if (manifest.compare(currentVersion) > 0)
        {
            _logger.info("FW-OTA", "Nova versão disponível.");
            return true;
        }
        else
        {
            _logger.info("FW-OTA", "Versão remota não é mais nova. Sem update.");
            return false;
        }
    }

    bool FirmwareUpdater::performOta(const ManifestInfo &manifest)
    {
        if (!manifest.valid)
            return false;

        if (WiFi.status() != WL_CONNECTED)
        {
            _logger.error("FW-OTA", "WiFi não conectado, não dá pra fazer OTA.");
            return false;
        }

        _logger.info("FW-OTA", "Iniciando OTA a partir de: %s", manifest.fullUrl.c_str());

        HTTPClient http;
        http.setTimeout(10000); // 10 segundos

        WiFiClientSecure httpsClient;
        WiFiClient httpClient;

        bool useHttps = isHttpsUrl(manifest.fullUrl) || _useHttps;

        if (useHttps)
        {
            httpsClient.setInsecure();
            if (!http.begin(httpsClient, String(manifest.fullUrl.c_str())))
            {
                _logger.error("FW-OTA", "Falha ao iniciar HTTPClient para OTA (HTTPS).");
                return false;
            }
        }
        else
        {
            if (!http.begin(httpClient, String(manifest.fullUrl.c_str())))
            {
                _logger.error("FW-OTA", "Falha ao iniciar HTTPClient para OTA (HTTP).");
                return false;
            }
        }

        int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK)
        {
            _logger.error("FW-OTA", "Erro HTTP ao baixar firmware. Code: %d", httpCode);
            _logger.error("FW-OTA", "Erro: %s", http.errorToString(httpCode).c_str());
            http.end();
            return false;
        }

        int contentLength = http.getSize();
        WiFiClient *stream = http.getStreamPtr();

        if (contentLength <= 0)
        {
            _logger.error("FW-OTA", "Tamanho de firmware inválido (contentLength <= 0).");
            http.end();
            return false;
        }

        // Confere com o size esperado do manifest (se > 0)
        if (manifest.size > 0 && (size_t)contentLength != manifest.size)
        {
            _logger.warn("FW-OTA", "Aviso: tamanho do HTTP difere do manifest. HTTP=%d / manifest=%d", contentLength, manifest.size);
        }

        float sizeMb = contentLength / (1024.0f * 1024.0f);
        // _logger.info("FW-OTA", "Tamanho do firmware: ", contentLength, " bytes (", sizeMb, " MB)");
        _logger.info("FW-OTA", "Tamanho do firmware: %.2f MB", sizeMb);


        if (!Update.begin(contentLength))
        {
            _logger.error("FW-OTA", "Update.begin() falhou.");
            http.end();
            return false;
        }

        bool shaEnabled = false;
        Sha256Context shaCtx;
        if (_verifySha256)
        {
            shaCtx.begin();
            shaEnabled = true;
        }

        const size_t bufferSize = 1024;
        uint8_t buffer[bufferSize];
        size_t totalWritten = 0;
        uint32_t lastLog = millis();

        while (totalWritten < (size_t)contentLength && http.connected())
        {
            size_t available = stream->available();
            if (available == 0)
            {
                delay(1);
                continue;
            }

            size_t toRead = available;
            if (toRead > bufferSize)
                toRead = bufferSize;

            int bytesRead = stream->readBytes(buffer, toRead);
            if (bytesRead <= 0)
            {
                delay(1);
                continue;
            }

            if (shaEnabled)
                shaCtx.update(buffer, bytesRead);

            size_t written = Update.write(buffer, bytesRead);
            if (written != (size_t)bytesRead)
            {
                _logger.error("FW-OTA", "Erro: bytes escritos != bytes lidos.");
                Update.end();
                http.end();
                return false;
            }

            totalWritten += written;

            if (millis() - lastLog > 2000)
            {
                lastLog = millis();
                float percent = (totalWritten * 100.0f) / contentLength;
                float writtenMb = totalWritten / (1024.0f * 1024.0f);
                float totalMb = contentLength / (1024.0f * 1024.0f);

                _logger.info("FW-OTA", "Progresso: %.2f / %.2f MB (%.2f%%)",
                             writtenMb,
                             totalMb,
                             percent);
            }
        }

        if (totalWritten != (size_t)contentLength)
        {
            _logger.error("FW-OTA", "Erro: total de bytes escritos (%d) difere do contentLength esperado (%d). Abortando OTA.", totalWritten, contentLength);
            Update.end();
            http.end();
            return false;
        }

        if (shaEnabled)
        {
            uint8_t hash[32];
            shaCtx.finish(hash);

            // Converte para hex std::string
            char hashHex[65];
            for (int i = 0; i < 32; ++i)
            {
                sprintf(&hashHex[i * 2], "%02x", hash[i]);
            }
            hashHex[64] = '\0';

            std::string computedHash = std::string(hashHex);
            std::string expectedHash = manifest.checksumValue;
            std::transform(expectedHash.begin(), expectedHash.end(), expectedHash.begin(),
                           [](unsigned char c)
                           { return static_cast<char>(std::tolower(c)); });

            _logger.info("FW-OTA", "SHA256 esperado: %s", expectedHash.c_str());
            _logger.info("FW-OTA", "SHA256 calculado: %s", computedHash.c_str());

            if (manifest.checksumType == "sha256" && expectedHash.length() == 64)
            {
                if (computedHash != expectedHash)
                {
                    _logger.error("FW-OTA", "Erro: SHA256 não confere. Abortando OTA.");
                    Update.end();
                    http.end();
                    return false;
                }
                else
                {
                    _logger.info("FW-OTA", "SHA256 validado com sucesso.");
                }
            }
            else
            {
                _logger.info("FW-OTA", "Checksum sha256 não configurado corretamente no manifest, ignorando validação.");
            }
        }

        if (!Update.end())
        {
            _logger.error("FW-OTA", "Update.end() falhou. Erro: %d", Update.getError());
            http.end();
            return false;
        }

        if (!Update.isFinished())
        {
            _logger.error("FW-OTA", "Update não finalizou corretamente.");
            http.end();
            return false;
        }

        _logger.info("FW-OTA", "OTA concluída com sucesso! Reiniciando...");
        http.end();
        
        _updateHasChecked = true;
        ESP.restart();
        return true;
    }

    void FirmwareUpdater::checkAndUpdate(settings::FirmwareConfig currentSettings)
    {
        configure(currentSettings.manifest,
                  currentSettings.url,
                  currentSettings.verify_sha256);
        _logger.debug("FW-OTA", "after configure: _manifestUrl='%s' _baseUrl='%s' verifySha256=%d",
                      _manifestUrl.c_str(),
                      _baseUrl.c_str(),
                      _verifySha256 ? 1 : 0);

        _logger.info("FW-OTA", "Verificando atualização de firmware...");

        ManifestInfo manifest = fetchManifest();
        if (!manifest.valid)
        {
            _logger.error("FW-OTA", "Manifest inválido ou não obtido. OTA abortada.");
            return;
        }

        if (!isRemoteNewer(manifest))
        {
            _updateHasChecked = true;
            return;
        }

        performOta(manifest);
    }
};
