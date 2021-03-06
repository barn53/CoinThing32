#include "main.h"
#include "display.h"
#include "events.h"
#include "finnhub.h"
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
std::atomic_bool readyFlag;
} // namespace cointhing

void setup()
{
    readyFlag = false;
    if (esp_reset_reason() == ESP_RST_POWERON) {
        stats.reset();
    } else if (esp_reset_reason() == ESP_RST_BROWNOUT) {
        stats.inc_brownout_counter();
    } else {
        stats.inc_crash_counter();
    }

    createHeartbeatTask();

    Serial.begin(115200);
    TraceFunction;

    SPIFFS.begin();

    display.begin();

    createEvents();
    setupWiFi();

    createHousekeepingTask();

    createGeckoTask();
    createFinnhubTask();
    createDisplayTask();

    createTimers();
    createServer();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}
