#pragma once

#ifdef ARDUINO_ARCH_ESP32

#include "Contracts/Connectivity/IEventLatch.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace iotsmartsys::platform::espressif::arduino
{

    class ArduinoEventLatch final : public iotsmartsys::core::IEventLatch
    {
    public:
        ArduinoEventLatch()
        {
            _eg = xEventGroupCreate();
        }

        void set(uint32_t bits) override
        {
            xEventGroupSetBits(_eg, static_cast<EventBits_t>(bits));
        }

        void clear(uint32_t bits) override
        {
            xEventGroupClearBits(_eg, static_cast<EventBits_t>(bits));
        }

        uint32_t get() const override
        {
            return static_cast<uint32_t>(xEventGroupGetBits(_eg));
        }

        bool waitAll(uint32_t requiredBits, int32_t timeoutMs) override
        {
            TickType_t to = (timeoutMs < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeoutMs);

            const EventBits_t got = xEventGroupWaitBits(
                _eg,
                static_cast<EventBits_t>(requiredBits),
                pdFALSE, // não limpa ao sair (latch)
                pdTRUE,  // espera TODOS os bits
                to);

            return (static_cast<uint32_t>(got) & requiredBits) == requiredBits;
        }

    private:
        EventGroupHandle_t _eg{};
    };

} // namespace iotsmartsys::platform::espressif::arduino

#endif // ARDUINO_ARCH_ESP32

#ifndef ARDUINO_ARCH_ESP32

// Provide a minimal stub for non-ESP32 builds so higher-level code can compile.
#include "Contracts/Connectivity/IEventLatch.h"
#include <Arduino.h>

namespace iotsmartsys::platform::espressif::arduino
{
    // A lightweight event latch implementation for platforms without FreeRTOS (e.g. ESP8266).
    // It stores bits in an atomic-like volatile and implements waitAll by polling with a timeout.
    class ArduinoEventLatch final : public iotsmartsys::core::IEventLatch
    {
    public:
        ArduinoEventLatch() : _bits(0) {}

        void set(uint32_t bits) override
        {
            // atomic-ish set
            noInterrupts();
            _bits |= bits;
            interrupts();
        }

        void clear(uint32_t bits) override
        {
            noInterrupts();
            _bits &= ~bits;
            interrupts();
        }

        uint32_t get() const override { return _bits; }

        bool waitAll(uint32_t requiredBits, int32_t timeoutMs) override
        {
            if (requiredBits == 0)
                return true;

            const unsigned long start = millis();
            while (true)
            {
                if ((get() & requiredBits) == requiredBits)
                    return true;

                if (timeoutMs >= 0 && (int32_t)(millis() - start) >= timeoutMs)
                    return false;

                // Yield to allow other tasks/handlers to run; short sleep to avoid tight busy loop
                delay(1);
            }
        }

    private:
        volatile uint32_t _bits;
    };
}

#endif // !ARDUINO_ARCH_ESP32