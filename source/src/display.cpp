#include "display.h"
#include "gecko.h"
#include "semaphores.h"

#include <ArduinoJson.h>

namespace cointhing {

Display::Display()
{
}

void Display::show()
{
    TRC_I_FUNC
    RecursiveMutexGuard guard(geckoDataMutex);
    const auto& data(gecko.getData(0));
    const auto& settings(gecko.getSettings());

    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 10);
    tft.print(settings.name(0));
}

TFT_eSPI tft;
Display display;
TaskHandle_t displayTaskHandle;

void displayTask(void*)
{
    while (true) {
        if (xSemaphoreTake(displaySemaphore, portMAX_DELAY)) {
            display.show();
        }
    }
}

void createDisplayTask()
{
    tft.begin();
    tft.setRotation(0); // 0 & 2 Portrait. 1 & 3 landscape
    tft.setTextWrap(false);
    tft.fillScreen(TFT_GREEN);

    xTaskCreate(
        displayTask, /* Task function. */
        "displayTask", /* name of task. */
        10000, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &displayTaskHandle /* Task handle to keep track of created task */);
}

} // namespace cointhing
