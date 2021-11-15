#include "display.h"
#include "events.h"
#include "gecko.h"
#include "semaphores.h"

#include <ArduinoJson.h>

namespace cointhing {

Display::Display()
{
}

void Display::clear()
{
    tft.fillScreen(TFT_BLACK);
}

void Display::nextId()
{
    RecursiveMutexGuard guard(dataMutex);
    ++displayId;
    if (displayId >= gecko.getValues().size()) {
        displayId = 0;
    }
}

void Display::show(bool next)
{
    TRC_I_FUNC
    RecursiveMutexGuard guard(dataMutex);
    if (!gecko.valid()) {
        return;
    }

    if (next) {
        nextId();
    }

    for (const auto& c : gecko.getChartData()) {
        TRC_I_PRINTF("Coin: %s - values: %u\n", c.first.c_str(), c.second.size());
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 10);

    tft.loadFont(F("NotoSans-Regular20"));
    String msg(gecko.getSettings().name(displayId));
    msg += ": ";
    msg += gecko.getValues()[displayId].priceCurrency1;
    msg += gecko.getSettings().currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, 10, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 40);
    msg = "24h change: ";
    msg += gecko.getValues()[displayId].change24hCurrency1;
    msg += "%";
    tft.fillRect(tft.textWidth(msg) - 5, 40, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 70);
    msg = "Cap: ";
    msg += gecko.getValues()[displayId].marketCapCurrency1;
    msg += gecko.getSettings().currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, 70, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 100);
    msg = "24h vol: ";
    msg += gecko.getValues()[displayId].volume24hCurrency1;
    msg += gecko.getSettings().currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, 100, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 130);
    msg = "chart: ";
    msg += gecko.getChartData().size();
    tft.fillRect(tft.textWidth(msg) - 5, 130, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.unloadFont();
}

TFT_eSPI tft;
Display display;
TaskHandle_t displayTaskHandle;

void displayTask(void*)
{
    display.clear();
    while (true) {
        if (xSemaphoreTake(displayNextSemaphore, portMAX_DELAY)) {
            display.show(true);
        }
        xSemaphoreTake(displayNextSemaphore, 0); // consume semaphore if given to avoid immediate rerun
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
        TASK_STACK_SIZE, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &displayTaskHandle /* Task handle to keep track of created task */);
}

} // namespace cointhing
