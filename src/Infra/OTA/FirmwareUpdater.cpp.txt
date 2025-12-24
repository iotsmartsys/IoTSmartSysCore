#include "FirmwareUpdater.h"
#if defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Updater.h>
#include <BearSSLHelpers.h>
#endif
#include "Infra/Settings/VersionInfo.h"
#include "Infra/Utils/Logger.h"
#if defined(ESP32)
#include "mbedtls/sha256.h"
#endif
#include <cstring>

using namespace OTA;

static const int FW_OTA_MAX_HTTP_RETRIES = 3;

#if defined(ESP32)
struct Sha256Context
{
    mbedtls_sha256_context ctx;
    void begin()
    {
        mbedtls_sha256_init(&ctx);
        mbedtls_sha256_starts_ret(&ctx, 0);
    }
    void update(const uint8_t *data, size_t len)
    {
        mbedtls_sha256_update_ret(&ctx, data, len);
    }
    void finish(uint8_t out[32])
    {
        mbedtls_sha256_finish_ret(&ctx, out);
        mbedtls_sha256_free(&ctx);
    }
};
#elif defined(ESP8266)
struct Sha256Context
{
    BearSSL::HashSHA256 hasher;
    void begin()
    {
        hasher.begin();
    }
    void update(const uint8_t *data, size_t len)
    {
        hasher.add(data, len);
    }
    void finish(uint8_t out[32])
    {
        hasher.end();
        memcpy(out, hasher.hash(), hasher.len());
    }
};
#endif

static bool isHttpsUrl(const String &url)
{
    return url.startsWith("https://");
}

static bool isSpace(char c)
{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

// Lê um valor string: "key": "valor"
static String jsonGetString(const String &json, const char *key)
{
    String keyPattern = String("\"") + key + "\"";
    int keyPos = json.indexOf(keyPattern);
    if (keyPos < 0)
        return "";

    int colonPos = json.indexOf(':', keyPos + keyPattern.length());
    if (colonPos < 0)
        return "";

    int i = colonPos + 1;
    while (i < (int)json.length() && isSpace(json[i]))
        i++;
    if (i >= (int)json.length())
        return "";

    // Se for string com aspas
    if (json[i] == '"')
    {
        i++; // pula a primeira aspa
        String out;
        while (i < (int)json.length())
        {
            char c = json[i];
            if (c == '\\' && i + 1 < (int)json.length())
            {
                // trata escape básico: \" \\ etc.
                i++;
                c = json[i];
            }
            else if (c == '"')
            {
                break;
            }
            out += c;
            i++;
        }
        return out;
    }

    // Caso não seja string (ex: número, true/false) devolve o token cru
    int start = i;
    while (i < (int)json.length() && !isSpace(json[i]) &&
           json[i] != ',' && json[i] != '}' && json[i] != ']')
    {
        i++;
    }
    return json.substring(start, i);
}

// Lê um valor inteiro: "key": 123
static long jsonGetLong(const String &json, const char *key)
{
    String raw = jsonGetString(json, key);
    raw.trim();
    if (!raw.length())
        return 0;
    return raw.toInt(); // suficiente para size, etc.
}

// Lê um booleano: "key": true / false
static bool jsonGetBool(const String &json, const char *key)
{
    String raw = jsonGetString(json, key);
    raw.trim();
    raw.toLowerCase();
    return raw == "true";
}

// Extrai um sub-bloco: ex: o objeto "checksum": { ... }
static String jsonGetObjectBlock(const String &json, const char *key)
{
    String keyPattern = String("\"") + key + "\"";
    int keyPos = json.indexOf(keyPattern);
    if (keyPos < 0)
        return "";

    int bracePos = json.indexOf('{', keyPos + keyPattern.length());
    if (bracePos < 0)
        return "";

    int depth = 0;
    int i = bracePos;
    for (; i < (int)json.length(); ++i)
    {
        if (json[i] == '{')
            depth++;
        else if (json[i] == '}')
        {
            depth--;
            if (depth == 0)
            {
                // inclui a chave de fechamento
                return json.substring(bracePos, i + 1);
            }
        }
    }
    return "";
}

FirmwareUpdater::FirmwareUpdater(const String &manifestUrl)
    : _manifestUrl(manifestUrl) {}

FirmwareUpdater::FirmwareUpdater()
{
}

static long long parseVersionNumber(const String &v)
{
    String digits;
    digits.reserve(v.length());
    for (size_t i = 0; i < v.length(); ++i)
    {
        char c = v[i];
        if (c >= '0' && c <= '9')
        {
            digits += c;
        }
    }
    if (digits.length() == 0)
        return 0;
    return atoll(digits.c_str());
}

FirmwareUpdater::ManifestInfo FirmwareUpdater::fetchManifest()
{
    ManifestInfo info;

    if (WiFi.status() != WL_CONNECTED)
    {
        LOG_PRINTLN(F("[FW-OTA] WiFi não conectado, pulando fetch do manifest."));
        return info;
    }

    if (_manifestUrl.length() == 0)
    {
        LOG_PRINTLN(F("[FW-OTA] Manifest URL vazia. Abortando fetch do manifest."));
        return info;
    }

    LOG_PRINT(F("[FW-OTA] Baixando manifest em: "));
    LOG_PRINTLN(_manifestUrl);

    String payload;

    for (int attempt = 1; attempt <= FW_OTA_MAX_HTTP_RETRIES; ++attempt)
    {
        HTTPClient http;
        http.setTimeout(10000); // 10 segundos

        WiFiClientSecure httpsClient;
        WiFiClient httpClient;

        bool useHttps = isHttpsUrl(_manifestUrl) || _useHttps;

        if (useHttps)
        {
            LOG_PRINTLN(F("[FW-OTA] Usando HTTPS para baixar o manifest."));
            httpsClient.setInsecure();

            if (!http.begin(httpsClient, _manifestUrl))
            {
                LOG_PRINTLN(F("[FW-OTA] Falha em iniciar HTTPClient (HTTPS)."));
                http.end();
            }
            else
            {
                int httpCode = http.GET();
                if (httpCode == HTTP_CODE_OK)
                {
                    payload = http.getString();
                    http.end();
                    break;
                }

                LOG_ERROR(F("[FW-OTA] Erro ao baixar manifest (HTTPS). HTTP code: "));
                LOG_ERROR(httpCode);
                LOG_ERROR(F(" ("));
                LOG_ERROR(http.errorToString(httpCode));
                LOG_ERROR(F(")"));
                http.end();
            }
        }
        else
        {
            LOG_PRINTLN(F("[FW-OTA] Usando HTTP para baixar o manifest."));

            if (!http.begin(httpClient, _manifestUrl))
            {
                LOG_PRINTLN(F("[FW-OTA] Falha em iniciar HTTPClient (HTTP)."));
                http.end();
            }
            else
            {
                int httpCode = http.GET();
                if (httpCode == HTTP_CODE_OK)
                {
                    payload = http.getString();
                    http.end();
                    break;
                }

                LOG_ERROR(F("[FW-OTA] Erro ao baixar manifest (HTTP). HTTP code: "));
                LOG_ERROR(httpCode);
                LOG_ERROR(F(" ("));
                LOG_ERROR(http.errorToString(httpCode));
                LOG_ERROR(F(")"));
                http.end();
            }
        }

        if (attempt < FW_OTA_MAX_HTTP_RETRIES)
        {
            LOG_PRINT(F("[FW-OTA] Tentando novamente fetch do manifest (tentativa "));
            LOG_PRINT(attempt + 1);
            LOG_PRINTLN(F(")..."));
            delay(500);
        }
    }

    if (!payload.length())
    {
        LOG_PRINTLN(F("[FW-OTA] Falha ao obter manifest após múltiplas tentativas."));
        return info;
    }

    LOG_PRINTLN(F("[FW-OTA] Manifest recebido:"));
    LOG_PRINTLN(payload);

    // 🔹 mapeando para o formato que você definiu:

    info.module = jsonGetString(payload, "module");
    info.env = jsonGetString(payload, "env");
    info.version = jsonGetString(payload, "version");
    info.urlPath = jsonGetString(payload, "urlPath");
    info.size = (size_t)jsonGetLong(payload, "size");
    info.mandatory = jsonGetBool(payload, "mandatory");

    // bloco "checksum": { "type": "...", "value": "..." }
    String checksumBlock = jsonGetObjectBlock(payload, "checksum");
    if (checksumBlock.length())
    {
        info.checksumType = jsonGetString(checksumBlock, "type");
        info.checksumValue = jsonGetString(checksumBlock, "value");
    }

    if (info.version.length() == 0 || info.urlPath.length() == 0)
    {
        LOG_PRINTLN(F("[FW-OTA] Manifest inválido: sem version ou urlPath."));
        return info;
    }

    // Monta URL completa:
    if (info.urlPath.startsWith("http://") || info.urlPath.startsWith("https://"))
    {
        info.fullUrl = info.urlPath;
    }
    else
    {
        String base = _baseUrl;
        info.fullUrl = base + info.urlPath;
    }

    info.valid = true;
    return info;
}

void FirmwareUpdater::configure(const String &manifestUrl,
                                const String &baseUrl,
                                bool verifySha256)
{
    LOG_PRINTLN(F("[FW-OTA] Configurando FirmwareUpdater..."));

    if (manifestUrl.length())
        _manifestUrl = manifestUrl;
    LOG_PRINTLN(F("[FW-OTA] Manifest URL: "));
    LOG_PRINT(_manifestUrl);
    if (baseUrl.length())
        _baseUrl = baseUrl;
    LOG_PRINTLN(F("[FW-OTA] Base URL: "));
    LOG_PRINT(_baseUrl);

    // Decide HTTPS com base tanto na baseUrl quanto na manifestUrl.
    _useHttps = isHttpsUrl(_baseUrl) || isHttpsUrl(_manifestUrl);

    LOG_PRINTLN(F("[FW-OTA] Usando HTTPS: "));
    LOG_PRINT(String(_useHttps ? "true" : "false"));
    _verifySha256 = verifySha256;
}

bool FirmwareUpdater::isRemoteNewer(const String &remoteVersion)
{
    String currentVersion = getBuildIdentifier();
    long long current = parseVersionNumber(currentVersion);
    long long remote = parseVersionNumber(remoteVersion);

    LOG_PRINT(F("[FW-OTA] Versão atual: "));
    LOG_PRINTLN(currentVersion);
    LOG_PRINT(F("[FW-OTA] Versão remota: "));
    LOG_PRINTLN(remoteVersion);

    if (remote > current)
    {
        LOG_PRINTLN(F("[FW-OTA] Nova versão disponível."));
        return true;
    }
    else
    {
        LOG_PRINTLN(F("[FW-OTA] Versão remota não é mais nova. Sem update."));
        return false;
    }
}

bool FirmwareUpdater::performOta(const ManifestInfo &manifest)
{
    if (!manifest.valid)
        return false;

    if (WiFi.status() != WL_CONNECTED)
    {
        LOG_PRINTLN(F("[FW-OTA] WiFi não conectado, não dá pra fazer OTA."));
        return false;
    }

    Serial.print(F("[FW-OTA] Iniciando OTA a partir de: "));
    LOG_PRINTLN(manifest.fullUrl);

    HTTPClient http;
    http.setTimeout(10000); // 10 segundos

    WiFiClientSecure httpsClient;
    WiFiClient httpClient;

    bool useHttps = isHttpsUrl(manifest.fullUrl) || _useHttps;

    if (useHttps)
    {
        httpsClient.setInsecure();
        if (!http.begin(httpsClient, manifest.fullUrl))
        {
            LOG_PRINTLN(F("[FW-OTA] Falha ao iniciar HTTPClient para OTA (HTTPS)."));
            return false;
        }
    }
    else
    {
        if (!http.begin(httpClient, manifest.fullUrl))
        {
            LOG_PRINTLN(F("[FW-OTA] Falha ao iniciar HTTPClient para OTA (HTTP)."));
            return false;
        }
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK)
    {
        Serial.print(F("[FW-OTA] Erro HTTP ao baixar firmware. Code: "));
        LOG_PRINTLN(httpCode);
        LOG_PRINT(F("[FW-OTA] Descrição do erro: "));
        LOG_PRINTLN(http.errorToString(httpCode));
        http.end();
        return false;
    }

    int contentLength = http.getSize();
    WiFiClient *stream = http.getStreamPtr();

    if (contentLength <= 0)
    {
        LOG_PRINTLN(F("[FW-OTA] Tamanho de firmware inválido (contentLength <= 0)."));
        http.end();
        return false;
    }

    // Confere com o size esperado do manifest (se > 0)
    if (manifest.size > 0 && (size_t)contentLength != manifest.size)
    {
        LOG_PRINT(F("[FW-OTA] Aviso: tamanho do HTTP difere do manifest. HTTP="));
        LOG_PRINT(contentLength);
        LOG_PRINT(F(" / manifest="));
        LOG_PRINTLN(manifest.size);
    }

    float sizeMb = contentLength / (1024.0f * 1024.0f);
    LOG_PRINT(F("[FW-OTA] Tamanho do firmware: "));
    LOG_PRINT(contentLength);
    LOG_PRINT(F(" bytes ("));
    LOG_PRINT(sizeMb); // duas casas decimais em MB
    LOG_PRINTLN(F(" MB)"));

    if (!Update.begin(contentLength))
    {
        LOG_PRINTLN(F("[FW-OTA] Update.begin() falhou."));
        http.end();
        return false;
    }

    bool shaEnabled = false;
#if defined(ESP32) || defined(ESP8266)
    Sha256Context shaCtx;
    if (_verifySha256)
    {
        shaCtx.begin();
        shaEnabled = true;
    }
#endif

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
            LOG_PRINTLN(F("[FW-OTA] Erro: bytes escritos != bytes lidos."));
            Update.end();
            http.end();
            return false;
        }

        totalWritten += written;

        if (millis() - lastLog > 2000)
        {
            lastLog = millis();
            LOG_PRINT(F("[FW-OTA] Progresso: "));
            LOG_PRINT((100 * totalWritten) / contentLength);
            LOG_PRINTLN(F("%"));
        }
    }

    if (totalWritten != (size_t)contentLength)
    {
        LOG_PRINT(F("[FW-OTA] Erro: total de bytes escritos ("));
        LOG_PRINT(totalWritten);
        LOG_PRINT(F(") difere do contentLength esperado ("));
        LOG_PRINT(contentLength);
        LOG_PRINTLN(F("). Abortando OTA."));
        Update.end();
        http.end();
        return false;
    }

    if (shaEnabled)
    {
        uint8_t hash[32];
        shaCtx.finish(hash);

        // Converte para hex string
        char hashHex[65];
        for (int i = 0; i < 32; ++i)
        {
            sprintf(&hashHex[i * 2], "%02x", hash[i]);
        }
        hashHex[64] = '\0';

        String computedHash = String(hashHex);
        String expectedHash = manifest.checksumValue;
        expectedHash.toLowerCase();

        Serial.print(F("[FW-OTA] SHA256 esperado:  "));
        LOG_PRINTLN(expectedHash);
        Serial.print(F("[FW-OTA] SHA256 calculado: "));
        LOG_PRINTLN(computedHash);

        if (manifest.checksumType == "sha256" && expectedHash.length() == 64)
        {
            if (computedHash != expectedHash)
            {
                LOG_PRINTLN(F("[FW-OTA] Erro: SHA256 não confere. Abortando OTA."));
                Update.end();
                http.end();
                return false;
            }
            else
            {
                LOG_PRINTLN(F("[FW-OTA] SHA256 validado com sucesso."));
            }
        }
        else
        {
            LOG_PRINTLN(F("[FW-OTA] Checksum sha256 não configurado corretamente no manifest, ignorando validação."));
        }
    }

    if (!Update.end())
    {
        LOG_PRINT(F("[FW-OTA] Update.end() falhou. Erro: "));
        LOG_PRINTLN(Update.getError());
        http.end();
        return false;
    }

    if (!Update.isFinished())
    {
        LOG_PRINTLN(F("[FW-OTA] Update não finalizou corretamente."));
        http.end();
        return false;
    }

    LOG_PRINTLN(F("[FW-OTA] OTA concluída com sucesso! Reiniciando..."));
    http.end();
    delay(1000);
    ESP.restart();
    return true;
}

void FirmwareUpdater::checkAndUpdate()
{
    LOG_PRINTLN(F("[FW-OTA] Verificando atualização de firmware..."));

    ManifestInfo manifest = fetchManifest();
    if (!manifest.valid)
    {
        LOG_PRINTLN(F("[FW-OTA] Manifest inválido ou não obtido. OTA abortada."));
        return;
    }

    if (!isRemoteNewer(manifest.version))
    {
        return;
    }

    performOta(manifest);
}

OTA::FirmwareUpdater fwUpdater;
