#include "Config/BuildConfig.h"

#if IOTSMARTSYS_SCREEN_CONSOLE_ENABLED

#include "Platform/Arduino/Display/ST7789ScreenConsole.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

namespace iotsmartsys::platform::arduino
{
    ST7789ScreenConsole::ST7789ScreenConsole(const ST7789ScreenConsoleConfig &config)
        : config_(config),
          spi_(),
          display_(&spi_, config.csPin, config.dcPin, config.resetPin)
    {
    }

    void ST7789ScreenConsole::begin()
    {
        if (config_.backlightPin >= 0)
        {
            pinMode(config_.backlightPin, OUTPUT);
            digitalWrite(config_.backlightPin, config_.backlightActiveHigh ? HIGH : LOW);
        }

        spi_.begin(config_.clockPin, -1, config_.mosiPin, config_.csPin);
        display_.init(config_.nativeWidth, config_.nativeHeight);
        display_.setRotation(config_.rotation);
        display_.setTextSize(config_.textSize == 0 ? 1 : config_.textSize);
        display_.setTextWrap(false);
        display_.fillScreen(config_.backgroundColor);

        const std::uint16_t textSize = config_.textSize == 0 ? 1 : config_.textSize;
        lineHeight_ = static_cast<std::uint16_t>((8U * textSize) + config_.lineSpacing);
        if (lineHeight_ == 0)
        {
            lineHeight_ = 1;
        }

        const std::int16_t width = display_.width();
        const std::int32_t usableWidth =
            static_cast<std::int32_t>(width) - (2L * config_.horizontalMargin);
        const std::uint16_t characterWidth = static_cast<std::uint16_t>(6U * textSize);
        charactersPerLine_ = usableWidth > 0
                                 ? std::max<std::size_t>(1, static_cast<std::size_t>(usableWidth) / characterWidth)
                                 : 1;

        const std::int16_t height = display_.height();
        visibleCapacity_ = height > 0
                               ? std::max<std::size_t>(1, static_cast<std::size_t>(height) / lineHeight_)
                               : 1;
        visibleCapacity_ = std::min<std::size_t>(visibleCapacity_, kHistoryCapacity);

        head_ = 0;
        count_ = 0;
        ready_ = true;
    }

    void ST7789ScreenConsole::writef(iotsmartsys::core::ScreenColor color, const char *fmt, va_list args)
    {
        if (!ready_ || fmt == nullptr)
        {
            return;
        }

        char formatted[kTextCapacity];
        const int result = vsnprintf(formatted, sizeof(formatted), fmt, args);
        if (result < 0)
        {
            return;
        }

        appendWrapped(formatted, color);
        render();
    }

    void ST7789ScreenConsole::clear()
    {
        head_ = 0;
        count_ = 0;
        if (ready_)
        {
            display_.fillScreen(config_.backgroundColor);
        }
    }

    void ST7789ScreenConsole::appendWrapped(
        const char *text,
        iotsmartsys::core::ScreenColor color)
    {
        if (text == nullptr || text[0] == '\0')
        {
            appendLine("", 0, color);
            return;
        }

        const char *cursor = text;
        while (*cursor != '\0')
        {
            if (*cursor == '\n')
            {
                appendLine("", 0, color);
                ++cursor;
                continue;
            }

            std::size_t length = 0;
            while (cursor[length] != '\0' && cursor[length] != '\n' &&
                   length < charactersPerLine_)
            {
                ++length;
            }

            appendLine(cursor, length, color);
            cursor += length;

            if (*cursor == '\n')
            {
                ++cursor;
                if (*cursor == '\0')
                {
                    appendLine("", 0, color);
                }
            }
        }
    }

    void ST7789ScreenConsole::appendLine(
        const char *text,
        std::size_t length,
        iotsmartsys::core::ScreenColor color)
    {
        length = std::min<std::size_t>(length, kTextCapacity - 1);

        std::size_t index = 0;
        if (count_ < visibleCapacity_)
        {
            index = (head_ + count_) % kHistoryCapacity;
            ++count_;
        }
        else
        {
            index = head_;
            head_ = (head_ + 1) % kHistoryCapacity;
        }

        std::memcpy(lines_[index].text, text, length);
        lines_[index].text[length] = '\0';
        lines_[index].color = color;
    }

    void ST7789ScreenConsole::render()
    {
        if (count_ == 0)
        {
            return;
        }

        const std::int16_t width = display_.width();
        const std::int16_t height = display_.height();
        const std::int16_t margin = static_cast<std::int16_t>(
            std::min<std::uint16_t>(config_.horizontalMargin, static_cast<std::uint16_t>(std::max<std::int16_t>(0, width))));
        const std::int16_t bandWidth = std::max<std::int16_t>(0, width - (2 * margin));
        const std::int16_t startY = static_cast<std::int16_t>(
            std::max<std::int32_t>(0, static_cast<std::int32_t>(height) -
                                          static_cast<std::int32_t>(count_ * lineHeight_)));

        for (std::size_t position = 0; position < count_; ++position)
        {
            const std::size_t index = (head_ + position) % kHistoryCapacity;
            const std::int16_t y = static_cast<std::int16_t>(startY + (position * lineHeight_));
            display_.fillRect(margin, y, bandWidth, lineHeight_, config_.backgroundColor);
            display_.setCursor(margin, y);
            display_.setTextColor(toNativeColor(lines_[index].color), config_.backgroundColor);
            display_.print(lines_[index].text);
        }
    }

    std::uint16_t ST7789ScreenConsole::toNativeColor(iotsmartsys::core::ScreenColor color) const
    {
        using iotsmartsys::core::ScreenColor;
        switch (color)
        {
        case ScreenColor::Default:
            return config_.foregroundColor;
        case ScreenColor::White:
            return ST77XX_WHITE;
        case ScreenColor::Red:
            return ST77XX_RED;
        case ScreenColor::Green:
            return ST77XX_GREEN;
        case ScreenColor::Blue:
            return ST77XX_BLUE;
        case ScreenColor::Yellow:
            return ST77XX_YELLOW;
        case ScreenColor::Cyan:
            return ST77XX_CYAN;
        case ScreenColor::Magenta:
            return ST77XX_MAGENTA;
        }
        return config_.foregroundColor;
    }
} // namespace iotsmartsys::platform::arduino

#endif // IOTSMARTSYS_SCREEN_CONSOLE_ENABLED
