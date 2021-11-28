#include "timers.h"
#include "display.h"
#include "finnhub.h"
#include "gecko.h"
#include "main.h"
#include "tasks.h"
#include "tracer.h"

namespace cointhing {

void createTimers()
{
    TraceFunction;

    TimerHandle_t fetchGeckoPriceTimer(xTimerCreate("fetchGeckoPriceTimer", (20 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        if (readyFlag) {
            GeckoRemit type(GeckoRemit::fetchPrices);
            xQueueSend(geckoQueue, static_cast<void*>(&type), 0);
        }
    }));
    xTimerStart(fetchGeckoPriceTimer, 0);

    TimerHandle_t fetchGeckoChartTimer(xTimerCreate("fetchGeckoChartTimer", (15 * 60 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        if (readyFlag) {
            GeckoRemit type(GeckoRemit::fetchCharts);
            xQueueSend(geckoQueue, static_cast<void*>(&type), 0);
        }
    }));
    xTimerStart(fetchGeckoChartTimer, 0);

    TimerHandle_t fetchFinnhubPriceTimer(xTimerCreate("fetchFinnhubPriceTimer", (20 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        if (readyFlag) {
            FinnhubRemit type(FinnhubRemit::fetchPrices);
            xQueueSend(finnhubQueue, static_cast<void*>(&type), 0);
        }
    }));
    xTimerStart(fetchFinnhubPriceTimer, 0);

    TimerHandle_t fetchFinnhubChartTimer(xTimerCreate("fetchFinnhubChartTimer", (15 * 60 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        if (readyFlag) {
            FinnhubRemit type(FinnhubRemit::fetchCharts);
            xQueueSend(finnhubQueue, static_cast<void*>(&type), 0);
        }
    }));
    xTimerStart(fetchFinnhubChartTimer, 0);

    TimerHandle_t displayNextTimer(xTimerCreate("displayNextTimer", (2 * 1000) / portTICK_PERIOD_MS, pdTRUE, nullptr, [](TimerHandle_t xTimer) {
        if (readyFlag) {
            xTaskNotify(displayTaskHandle, static_cast<uint32_t>(DisplayNotificationType::showNextId), eSetValueWithOverwrite);
        }
    }));
    xTimerStart(displayNextTimer, 0);

    // based on settings:
    // xTimerChangePeriod()
}

} // namespace cointhing
