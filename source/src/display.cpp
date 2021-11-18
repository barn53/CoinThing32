#include "display.h"
#include "events.h"
#include "gecko.h"
#include "main.h"
#include "settings.h"
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
    m_display_coin_index = 0;
}

void Display::nextCoinId()
{
    ++m_display_coin_index;
    if (m_display_coin_index >= gecko.getCoinPrices().size()) {
        m_display_coin_index = 0;
    }
}

void Display::show() const
{
    TRC_I_FUNC
    if (!gecko.valid()) {
        m_clear_on_show = true;
        return;
    }

    if (m_clear_on_show) {
        clear();
        m_clear_on_show = false;
    }

    TRC_I_PRINTF("Show coin ID: %u\n", m_display_coin_index);
    for (const auto& c : gecko.getChartData()) {
        TRC_I_PRINTF("Coin: %s - values: %u\n", c.first.c_str(), c.second.size());
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(0, 10);
    tft.loadFont(F("NotoSans-Regular20"));

    const auto& settingsCoins(gecko.getSettingsCoins());
    const auto& coinPrices(gecko.getCoinPrices()[m_display_coin_index]);

    String msg(settingsCoins.name(m_display_coin_index));
    msg += ": ";
    msg += coinPrices.priceCurrency1;
    msg += settingsCoins.currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, 10, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 40);
    msg = "24h change: ";
    msg += coinPrices.change24hCurrency1;
    msg += "%";
    tft.fillRect(tft.textWidth(msg) - 5, 40, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 70);
    msg = "Cap: ";
    msg += coinPrices.marketCapCurrency1;
    msg += settingsCoins.currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, 70, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 100);
    msg = "24h vol: ";
    msg += coinPrices.volume24hCurrency1;
    msg += settingsCoins.currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, 100, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 130);
    msg = "charts: ";
    msg += gecko.getChartData().size();
    tft.fillRect(tft.textWidth(msg) - 5, 130, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.setCursor(0, 160);
    msg = "fetches\nprice: ";
    msg += gecko.getCountPriceFetches();
    msg += ", chart: ";
    msg += gecko.getCountChartFetches();
    tft.fillRect(tft.textWidth(msg) - 5, 160, 240 - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
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
    vTaskDelay(4000);
    m_clear_on_show = true;
}

TFT_eSPI tft;
Display display;
TaskHandle_t displayTaskHandle;

void displayTask(void*)
{

    tft.begin();
    tft.setRotation(0); // 0 & 2 Portrait. 1 & 3 landscape
    tft.setTextWrap(false);

    display.clear();

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
                RecursiveMutexGuard g(geckoSyncMutex);
                display.show();
                display.nextCoinId();
                break;
            }
        }
    }
}

void createDisplayTask()
{
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
