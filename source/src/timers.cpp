#include "timers.h"
#include "display.h"
#include "gecko.h"
#include "tasks.h"

namespace cointhing {

void createTimers()
{
    TRC_I_FUNC
    TimerHandle_t fetchTimeTimer(xTimerCreate("fetchTimeTimer", (60 * 60 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xTaskNotify(housekeepingTaskHandle, static_cast<uint32_t>(HousekeepingNotificationType::fetchTime), eSetValueWithOverwrite);
    }));
    xTimerStart(fetchTimeTimer, 0);

    TimerHandle_t fetchPriceTimer(xTimerCreate("fetchPriceTimer", (20 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xTaskNotify(geckoTaskHandle, static_cast<uint32_t>(GeckoNotificationType::fetchPrices), eSetValueWithOverwrite);
    }));
    xTimerStart(fetchPriceTimer, 0);

    TimerHandle_t fetchChartTimer(xTimerCreate("fetchChartTimer", (15 * 60 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xTaskNotify(geckoTaskHandle, static_cast<uint32_t>(GeckoNotificationType::fetchCharts), eSetValueWithOverwrite);
    }));
    xTimerStart(fetchChartTimer, 0);

    TimerHandle_t displayNextTimer(xTimerCreate("displayNextTimer", (2 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xTaskNotify(displayTaskHandle, static_cast<uint32_t>(DisplayNotificationType::showNextId), eSetValueWithOverwrite);
    }));
    xTimerStart(displayNextTimer, 0);
}

} // namespace cointhing
