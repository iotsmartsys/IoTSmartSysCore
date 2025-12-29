#pragma once

#include "Contracts/Runtime/IGatedRoutine.h"
#include "Contracts/Connectivity/ConnectivityGate.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace iotsmartsys::platform::espressif
{

    class FreeRtosGatedRoutineRunner
    {
    public:
        static void start(iotsmartsys::core::IGatedRoutine &routine,
                          uint32_t stackBytes = 8192,
                          UBaseType_t priority = 5,
                          BaseType_t coreId = tskNO_AFFINITY)
        {
            
            auto taskFn = [](void *arg)
            {
                auto *r = static_cast<iotsmartsys::core::IGatedRoutine *>(arg);
                auto &gate = iotsmartsys::core::ConnectivityGate::instance();

                uint32_t backoffMs = 0;

                for (;;)
                {
                    // 1) Espera pré-requisitos (latch)
                    gate.waitBits(r->requiredBits(), -1);

                    // 2) Executa
                    auto res = r->execute();

                    // 3) Define próximo delay (intervalo ou backoff)
                    uint32_t delayMs = r->intervalMs();
                    if (res == iotsmartsys::core::RoutineResult::RetrySoon)
                    {
                        backoffMs = (backoffMs == 0) ? 5000 : (backoffMs * 2);
                        if (backoffMs > 10000)
                            backoffMs = 10000;
                        delayMs = backoffMs;
                    }
                    else if (res == iotsmartsys::core::RoutineResult::RetryLater)
                    {
                        backoffMs = (backoffMs == 0) ? 20000 : (backoffMs * 2);
                        if (backoffMs > 60000)
                            backoffMs = 60000;
                        delayMs = backoffMs;
                    }
                    else
                    {
                        backoffMs = 0;
                    }

                    // 4) Dorme (se a conectividade cair no meio, na próxima iteração vai voltar ao waitBits)
                    vTaskDelay(pdMS_TO_TICKS(delayMs));
                }
            };

            TaskHandle_t handle{};
            if (coreId == tskNO_AFFINITY)
            {
                xTaskCreate(taskFn, routine.name(), stackBytes, &routine, priority, &handle);
            }
            else
            {
                xTaskCreatePinnedToCore(taskFn, routine.name(), stackBytes, &routine, priority, &handle, coreId);
            }
        }
    };

} // namespace iotsmartsys::platform::espressif