#pragma once

namespace iotsmartsys::core
{

    class ILogger;
    class ITimeProvider;

    namespace settings
    {
        class IReadOnlySettingsProvider;
        class ISettingsGate;
    }

    class IServiceProvider
    {
    public:
        virtual ~IServiceProvider() = default;

        virtual ILogger *logger() const = 0;
        virtual ITimeProvider *time() const = 0;

        virtual settings::IReadOnlySettingsProvider *settings() const = 0;
        virtual settings::ISettingsGate *settingsGate() const = 0;
    };

} // namespace iotsmartsys::core