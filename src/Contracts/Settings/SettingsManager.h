#pragma once

#include <cstdint>
#include <string>
#include "Contracts/Common/StateResult.h"

#include "Contracts/Settings/Settings.h"
#include "Contracts/Settings/ISettingsFetcher.h"
#include "Contracts/Settings/ISettingsParser.h"
#include "Contracts/Providers/ISettingsProvider.h"
#include "Contracts/Settings/SettingsGate.h"
#include "Contracts/Logging/ILogger.h"
#include "Contracts/Settings/IReadOnlySettingsProvider.h"

namespace iotsmartsys::core::settings
{
    enum class SettingsManagerState : std::uint8_t
    {
        Idle,
        LoadedFromCache,
        FetchingFromApi,
        Ready,
        Error
    };

    struct SettingsManagerStats
    {
        std::uint32_t cache_load_ok{0};
        std::uint32_t cache_load_fail{0};
        std::uint32_t api_fetch_ok{0};
        std::uint32_t api_fetch_fail{0};
        std::uint32_t parse_fail{0};
        std::uint32_t nvs_save_fail{0};

        int last_http_status{-1};
        iotsmartsys::core::common::StateResult last_err{iotsmartsys::core::common::StateResult::Ok};
    };

    // Callback disparado quando settings atuais foram atualizados com sucesso a partir da API
    using SettingsUpdatedCallback = void (*)(const Settings &new_settings, void *user_ctx);

    class SettingsManager : public iotsmartsys::core::settings::IReadOnlySettingsProvider
    {
    public:
        SettingsManager(core::providers::ISettingsProvider &provider,
                        ISettingsFetcher &fetcher,
                        ISettingsParser &parser,
                        ISettingsGate &settingsGate);
        ~SettingsManager() override;
        // 1) Carrega do NVS se existir (rápido). Nunca bloqueia com rede.
        // Retorna:
        //  - ESP_OK: carregou do NVS
        //  - ESP_ERR_NVS_NOT_FOUND/ESP_FAIL/etc: não carregou, mas o manager continua funcional
        iotsmartsys::core::common::StateResult init();

        // 2) Dispara atualização via API (assíncrono; não trava firmware).
        // Use a API como fonte de verdade; se falhar, mantém cache atual.
        iotsmartsys::core::common::StateResult refreshFromApiAsync(const SettingsFetchRequest &req);

        // Cancela fetch se estiver rodando
        void cancel();

        // Estado/consulta
        bool hasCurrent() const;
        SettingsManagerState state() const;

        // Copia snapshot para o chamador (evita retornar referência com concorrência)
        bool copyCurrent(Settings &out) const;

        // Callback para “SettingsUpdated”
        void setUpdatedCallback(SettingsUpdatedCallback cb, void *user_ctx);

        // Estatísticas simples para debug/telemetria
        SettingsManagerStats stats() const;

        // Optional loop hook. Default is no-op so SettingsManager can be instantiated directly.
        void saveWiFiOnly(const WifiConfig &wifi);
        bool save(const Settings &settings);
        void handle();

    private:
        iotsmartsys::core::ILogger *_logger;
        static void onFetchCompletedStatic(const SettingsFetchResult &res, void *ctx);
        void onFetchCompleted(const SettingsFetchResult &res);

        void setState(SettingsManagerState s);
        void updateStatsFail(iotsmartsys::core::common::StateResult err, int http_status);

        core::providers::ISettingsProvider &_provider;
        ISettingsFetcher &_fetcher;
        ISettingsParser &_parser;

        // estado atual
        mutable void *_mutex; // SemaphoreHandle_t, mas sem incluir FreeRTOS aqui no Core
        SettingsManagerState _state{SettingsManagerState::Idle};
        bool _has_current{false};
        Settings _current{};
        SettingsManagerStats _stats{};

        // callback
        SettingsUpdatedCallback _updated_cb{nullptr};
        void *_updated_ctx{nullptr};

        iotsmartsys::core::settings::ISettingsGate &_settingsGate;
        // Keep last sync URL alive while the async fetcher is running.
        std::string _syncUrlBuffer{};

        void syncFromApi();
    };
} // namespace iotsmartsys::core::settings
