#include "timers.h"
#include "semaphores.h"

namespace cointhing {

TimerHandle_t fetchPriceTimer;
TimerHandle_t fetchChartTimer;
TimerHandle_t displayNextTimer;

void createTimers()
{
    fetchPriceTimer = xTimerCreate("fetchPriceTimer", (20 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xSemaphoreGive(fetchPriceSemaphore);
    });
    xTimerStart(fetchPriceTimer, 0);

    fetchChartTimer = xTimerCreate("fetchChartTimer", (15 * 60 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xSemaphoreGive(fetchChartSemaphore);
    });
    xTimerStart(fetchChartTimer, 0);

    displayNextTimer = xTimerCreate("displayNextTimer", (2 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xSemaphoreGive(displayNextSemaphore);
    });
    xTimerStart(displayNextTimer, 0);
}

} // namespace cointhing
