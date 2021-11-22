#include "timers.h"
#include "display.h"
#include "gecko.h"
#include "tasks.h"
#include "tracer.h"

namespace cointhing {

void createTimers()
{
    TraceFunction;

    TimerHandle_t fetchPriceTimer(xTimerCreate("fetchPriceTimer", (20 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        GeckoRemit type(GeckoRemit::fetchPrices);
        xQueueSend(geckoQueue, static_cast<void*>(&type), 0);
    }));
    xTimerStart(fetchPriceTimer, 0);

    TimerHandle_t fetchChartTimer(xTimerCreate("fetchChartTimer", (15 * 60 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        GeckoRemit type(GeckoRemit::fetchCharts);
        xQueueSend(geckoQueue, static_cast<void*>(&type), 0);
    }));
    xTimerStart(fetchChartTimer, 0);

    TimerHandle_t displayNextTimer(xTimerCreate("displayNextTimer", (2 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        xTaskNotify(displayTaskHandle, static_cast<uint32_t>(DisplayNotificationType::showNextId), eSetValueWithOverwrite);
    }));
    xTimerStart(displayNextTimer, 0);
}

} // namespace cointhing
