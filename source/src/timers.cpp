#include "timers.h"
#include "semaphores.h"

namespace cointhing {

TimerHandle_t fetchTimer;

void createTimers()
{
    fetchTimer = xTimerCreate("readTask", 10000 / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xSemaphoreGive(fetchSemaphore);
    });

    xTimerStart(fetchTimer, 0);
}

} // namespace cointhing
