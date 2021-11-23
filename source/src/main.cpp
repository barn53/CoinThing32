#include "main.h"
#include "display.h"
#include "events.h"
#include "gecko.h"
#include "stats.h"
#include "tasks.h"
#include "timers.h"
#include "tracer.h"
#include "web_server.h"
#include "wifi_utils.h"
#include <Arduino.h>
#include <SPIFFS.h>

using namespace cointhing;

namespace cointhing {
SemaphoreHandle_t coinsMutex = xSemaphoreCreateRecursiveMutex();
} // namespace cointhing

void setup()
{
    if (esp_reset_reason() == ESP_RST_POWERON) {
        stats.reset();
    } else if (esp_reset_reason() == ESP_RST_BROWNOUT) {
        stats.inc_brownout_counter();
    } else {
        stats.inc_crash_counter();
    }

    Serial.begin(115200);
    SPIFFS.begin();

    TraceFunction;

    createHeartbeatTask();

    createEventLoop();
    registerEventHandler();

    setupWiFi();

    createHousekeepingTask();

    createGeckoTask();
    createDisplayTask();

    createTimers();

    createServer();

    settings.read();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}
