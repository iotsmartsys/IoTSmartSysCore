#pragma once

#include "Config/BuildConfig.h"

#if IOTSMARTSYS_SCREEN_CONSOLE_ENABLED

#include "Contracts/Display/IScreenConsole.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#include <cstddef>
#include <cstdint>

namespace iotsmartsys::platform::arduino
{
    struct ST7789ScreenConsoleConfig
    {
        std::int8_t csPin;
        std::int8_t dcPin;
        std::int8_t resetPin;
        std::int8_t clockPin;
        std::int8_t mosiPin;
        std::int8_t backlightPin;
        bool backlightActiveHigh;
        std::uint16_t nativeWidth;
        std::uint16_t nativeHeight;
        std::uint8_t rotation;
        std::uint8_t textSize;
        std::uint16_t horizontalMargin;
        std::uint16_t lineSpacing;
        std::uint16_t backgroundColor;
        std::uint16_t foregroundColor;
    };

    class ST7789ScreenConsole final : public iotsmartsys::core::IScreenConsole
    {
    public:
        explicit ST7789ScreenConsole(const ST7789ScreenConsoleConfig &config);

        void begin() override;
        void writef(iotsmartsys::core::ScreenColor color, const char *fmt, va_list args) override;
        void clear() override;
        bool isReady() const override { return ready_; }

    private:
        static constexpr std::size_t kHistoryCapacity = 24;
        static constexpr std::size_t kTextCapacity = 256;

        struct Line
        {
            char text[kTextCapacity];
            iotsmartsys::core::ScreenColor color;
        };

        void appendWrapped(const char *text, iotsmartsys::core::ScreenColor color);
        void appendLine(const char *text, std::size_t length, iotsmartsys::core::ScreenColor color);
        void render();
        std::uint16_t toNativeColor(iotsmartsys::core::ScreenColor color) const;

        ST7789ScreenConsoleConfig config_;
        SPIClass spi_;
        Adafruit_ST7789 display_;
        Line lines_[kHistoryCapacity]{};
        std::size_t head_{0};
        std::size_t count_{0};
        std::size_t visibleCapacity_{1};
        std::size_t charactersPerLine_{1};
        std::uint16_t lineHeight_{1};
        bool ready_{false};
    };
} // namespace iotsmartsys::platform::arduino

#endif // IOTSMARTSYS_SCREEN_CONSOLE_ENABLED
