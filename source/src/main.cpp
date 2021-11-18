#include "main.h"
#include "display.h"
#include "events.h"
#include "gecko.h"
#include "tasks.h"
#include "timers.h"
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
    Serial.begin(115200);
    SPIFFS.begin();

    createEventLoop();
    registerEventHandler();

    setupWiFi();

    createGeckoTask();
    createDisplayTask();
    createHighWaterMarkTask();
    createHeartbeatTask();

    createTimers();

    createServer();

    settings.read();
}

void loop()
{
    vTaskDelay(portMAX_DELAY);
}
