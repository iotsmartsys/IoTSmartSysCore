#pragma once

#include <Arduino.h>
#include <cstdint>

#if defined(ARDUINO_ARCH_ESP32) && __has_include("soc/adc_channel.h")
#include "soc/adc_channel.h"
#endif

// Expand the official GPIO number before forming the public enumerator name.
#define IOTSSC_ADC1_GPIO_NAME_IMPL(pin) GPIO##pin
#define IOTSSC_ADC1_GPIO_NAME(pin) IOTSSC_ADC1_GPIO_NAME_IMPL(pin)

namespace iotsmartsys::platform::arduino
{
    // Physical GPIO numbers, selected by the SoC headers of this build.
    // ADC capability does not imply that a module exposes or leaves a pin free.
    enum class Esp32Adc1Pin : int
    {
#ifdef ADC1_CHANNEL_0_GPIO_NUM
        IOTSSC_ADC1_GPIO_NAME(ADC1_CHANNEL_0_GPIO_NUM) = ADC1_CHANNEL_0_GPIO_NUM,
#endif
#ifdef ADC1_CHANNEL_1_GPIO_NUM
        IOTSSC_ADC1_GPIO_NAME(ADC1_CHANNEL_1_GPIO_NUM) = ADC1_CHANNEL_1_GPIO_NUM,
#endif
#ifdef ADC1_CHANNEL_2_GPIO_NUM
        IOTSSC_ADC1_GPIO_NAME(ADC1_CHANNEL_2_GPIO_NUM) = ADC1_CHANNEL_2_GPIO_NUM,
#endif
#ifdef ADC1_CHANNEL_3_GPIO_NUM
        IOTSSC_ADC1_GPIO_NAME(ADC1_CHANNEL_3_GPIO_NUM) = ADC1_CHANNEL_3_GPIO_NUM,
#endif
#ifdef ADC1_CHANNEL_4_GPIO_NUM
        IOTSSC_ADC1_GPIO_NAME(ADC1_CHANNEL_4_GPIO_NUM) = ADC1_CHANNEL_4_GPIO_NUM,
#endif
#ifdef ADC1_CHANNEL_5_GPIO_NUM
        IOTSSC_ADC1_GPIO_NAME(ADC1_CHANNEL_5_GPIO_NUM) = ADC1_CHANNEL_5_GPIO_NUM,
#endif
#ifdef ADC1_CHANNEL_6_GPIO_NUM
        IOTSSC_ADC1_GPIO_NAME(ADC1_CHANNEL_6_GPIO_NUM) = ADC1_CHANNEL_6_GPIO_NUM,
#endif
#ifdef ADC1_CHANNEL_7_GPIO_NUM
        IOTSSC_ADC1_GPIO_NAME(ADC1_CHANNEL_7_GPIO_NUM) = ADC1_CHANNEL_7_GPIO_NUM,
#endif
#ifdef ADC1_CHANNEL_8_GPIO_NUM
        IOTSSC_ADC1_GPIO_NAME(ADC1_CHANNEL_8_GPIO_NUM) = ADC1_CHANNEL_8_GPIO_NUM,
#endif
#ifdef ADC1_CHANNEL_9_GPIO_NUM
        IOTSSC_ADC1_GPIO_NAME(ADC1_CHANNEL_9_GPIO_NUM) = ADC1_CHANNEL_9_GPIO_NUM,
#endif
    };

    constexpr bool esp32Adc1TargetSupported()
    {
#ifdef ADC1_CHANNEL_0_GPIO_NUM
        return true;
#else
        return false;
#endif
    }

    // Also usable at compile time when adapting a board's official pin symbol.
    constexpr bool isEsp32Adc1Gpio(int pin)
    {
        switch (pin)
        {
#ifdef ADC1_CHANNEL_0_GPIO_NUM
        case ADC1_CHANNEL_0_GPIO_NUM:
#endif
#ifdef ADC1_CHANNEL_1_GPIO_NUM
        case ADC1_CHANNEL_1_GPIO_NUM:
#endif
#ifdef ADC1_CHANNEL_2_GPIO_NUM
        case ADC1_CHANNEL_2_GPIO_NUM:
#endif
#ifdef ADC1_CHANNEL_3_GPIO_NUM
        case ADC1_CHANNEL_3_GPIO_NUM:
#endif
#ifdef ADC1_CHANNEL_4_GPIO_NUM
        case ADC1_CHANNEL_4_GPIO_NUM:
#endif
#ifdef ADC1_CHANNEL_5_GPIO_NUM
        case ADC1_CHANNEL_5_GPIO_NUM:
#endif
#ifdef ADC1_CHANNEL_6_GPIO_NUM
        case ADC1_CHANNEL_6_GPIO_NUM:
#endif
#ifdef ADC1_CHANNEL_7_GPIO_NUM
        case ADC1_CHANNEL_7_GPIO_NUM:
#endif
#ifdef ADC1_CHANNEL_8_GPIO_NUM
        case ADC1_CHANNEL_8_GPIO_NUM:
#endif
#ifdef ADC1_CHANNEL_9_GPIO_NUM
        case ADC1_CHANNEL_9_GPIO_NUM:
#endif
            return true;
        default:
            return false;
        }
    }

    inline bool isSupportedEsp32Adc1Pin(int pin)
    {
#if defined(ARDUINO_ARCH_ESP32)
        return pin >= 0 && pin < NUM_DIGITAL_PINS && pin <= UINT8_MAX &&
               isEsp32Adc1Gpio(pin) &&
               digitalPinToAnalogChannel(static_cast<std::uint8_t>(pin)) >= 0;
#else
        (void)pin;
        return false;
#endif
    }
}

#undef IOTSSC_ADC1_GPIO_NAME
#undef IOTSSC_ADC1_GPIO_NAME_IMPL
