#include "display.h"
#include "events.h"
#include "gecko.h"
#include "main.h"
#include "tasks.h"

#include <ArduinoJson.h>

namespace cointhing {

Display::Display()
{
}

void Display::clear() const
{
    tft.fillScreen(TFT_BLACK);
}

void Display::resetCoinId()
{
    RecursiveMutexGuard guard(coinsMutex);
    displayCoinIndex = 0;
}

void Display::nextCoinId()
{
    RecursiveMutexGuard guard(coinsMutex);
    ++displayCoinIndex;
    if (displayCoinIndex >= gecko.getCoinPrices().size()) {
        displayCoinIndex = 0;
    }
}

void Display::show() const
{
    TRC_I_FUNC
    RecursiveMutexGuard guard(coinsMutex);
    if (!gecko.valid()) {
        return;
    }

    for (const auto& c : gecko.getChartData()) {
        TRC_I_PRINTF("Coin: %s - values: %u\n", c.first.c_str(), c.second.size());
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 10);

    tft.loadFont(F("NotoSans-Regular20"));
    String msg(gecko.getSettings().name(displayCoinIndex));
    msg += ": ";
    msg += gecko.getCoinPrices()[displayCoinIndex].priceCurrency1;
    msg += gecko.getSettings().currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, 10, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 40);
    msg = "24h change: ";
    msg += gecko.getCoinPrices()[displayCoinIndex].change24hCurrency1;
    msg += "%";
    tft.fillRect(tft.textWidth(msg) - 5, 40, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 70);
    msg = "Cap: ";
    msg += gecko.getCoinPrices()[displayCoinIndex].marketCapCurrency1;
    msg += gecko.getSettings().currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, 70, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 100);
    msg = "24h vol: ";
    msg += gecko.getCoinPrices()[displayCoinIndex].volume24hCurrency1;
    msg += gecko.getSettings().currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, 100, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 130);
    msg = "chart: ";
    for (const auto& cd : gecko.getChartData()) {
        msg += cd.first;
        msg += "\n";
    }
    tft.print(msg);

    tft.unloadFont();
}

void Display::showNewSettings() const
{
    TRC_I_FUNC
    tft.loadFont(F("NotoSans-Regular20"));
    tft.fillScreen(TFT_GOLD);
    tft.setTextColor(TFT_WHITE, TFT_GOLD);
    tft.setCursor(10, 100);
    String msg("New Settings");
    tft.print(msg);
    tft.unloadFont();
    vTaskDelay(2000);
    clear();
}

TFT_eSPI tft;
Display display;
TaskHandle_t displayTaskHandle;

void displayTask(void*)
{
    DisplayNotificationType notificationType;

    while (true) {
        if (xTaskNotifyWait(0, 0xffffffff, reinterpret_cast<uint32_t*>(&notificationType), portMAX_DELAY)) {
            TRC_I_PRINTF("Notification type: %u\n", static_cast<uint32_t>(notificationType));
            switch (notificationType) {
            case DisplayNotificationType::settingsChanged:
                display.resetCoinId();
                display.showNewSettings();
                break;
            case DisplayNotificationType::showNextId:
                display.show();
                display.nextCoinId();
                break;
            }
        }
    }
}

void createDisplayTask()
{
    tft.begin();
    tft.setRotation(0); // 0 & 2 Portrait. 1 & 3 landscape
    tft.setTextWrap(false);

    display.clear();

    xTaskCreatePinnedToCore(
        displayTask, /* Task function. */
        "displayTask", /* name of task. */
        TASK_STACK_SIZE, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &displayTaskHandle, /* Task handle to keep track of created task */
        1);
}

} // namespace cointhing
