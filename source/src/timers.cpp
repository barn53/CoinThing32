#include "timers.h"
#include "display.h"
#include "gecko.h"

namespace cointhing {

TimerHandle_t fetchPriceTimer;
TimerHandle_t fetchChartTimer;
TimerHandle_t displayNextTimer;

void createTimers()
{
    fetchPriceTimer = xTimerCreate("fetchPriceTimer", (20 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xTaskNotify(displayTaskHandle, static_cast<uint32_t>(GeckoNotificationType::fetchPrices), eSetValueWithOverwrite);
    });
    // xTimerStart(fetchPriceTimer, 0);

    fetchChartTimer = xTimerCreate("fetchChartTimer", (15 * 60 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xTaskNotify(displayTaskHandle, static_cast<uint32_t>(GeckoNotificationType::fetchCharts), eSetValueWithOverwrite);
    });
    // xTimerStart(fetchChartTimer, 0);

    displayNextTimer = xTimerCreate("displayNextTimer", (2 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xTaskNotify(displayTaskHandle, static_cast<uint32_t>(DisplayNotificationType::showNextId), eSetValueWithOverwrite);
    });
    xTimerStart(displayNextTimer, 0);
}

} // namespace cointhing
