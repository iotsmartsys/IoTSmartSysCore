#pragma once
#include "Contracts/Connectivity/IEventLatch.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace iotsmartsys::platform::espressif
{

    class FreeRtosEventLatch final : public iotsmartsys::core::IEventLatch
    {
    public:
        FreeRtosEventLatch() { _eg = xEventGroupCreate(); }

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
            auto got = xEventGroupWaitBits(
                _eg,
                static_cast<EventBits_t>(requiredBits),
                pdFALSE, // não limpa (latch)
                pdTRUE,  // espera todos
                to);
            return (static_cast<uint32_t>(got) & requiredBits) == requiredBits;
        }

    private:
        EventGroupHandle_t _eg{};
    };

} // namespace iotsmartsys::platform::espressif