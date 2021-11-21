#include "display.h"
#include "events.h"
#include "gecko.h"
#include "main.h"
#include "settings.h"
#include "stats.h"
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
    tft.loadFont(F("NotoSans-Regular20"));

    const auto& geckoSettings(gecko.getSettings());
    const auto& coinPrices(gecko.getCoinPrices()[m_display_coin_index]);

    String msg(geckoSettings.name(m_display_coin_index));
    int16_t msgY(10);
    tft.setCursor(0, msgY);
    msg += ": ";
    msg += coinPrices.priceCurrency1;
    msg += geckoSettings.currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    msgY += 30;
    tft.setCursor(0, msgY);
    msg = "24h change: ";
    msg += coinPrices.change24hCurrency1;
    msg += "%";
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    msgY += 30;
    tft.setCursor(0, msgY);
    msg = "Cap: ";
    msg += coinPrices.marketCapCurrency1;
    msg += geckoSettings.currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    msgY += 30;
    tft.setCursor(0, msgY);
    msg = "24h vol: ";
    msg += coinPrices.volume24hCurrency1;
    msg += geckoSettings.currency1Symbol();
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    msgY += 30;
    tft.setCursor(0, msgY);
    msg = "charts: ";
    msg += gecko.getChartData().size();
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    msgY += 30;
    tft.setCursor(0, msgY);
    msg = stats.localTime();
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    msgY += 30;
    tft.setCursor(0, msgY);
    msg = "lpf: ";
    msg += timeFromTimestamp(stats.get_last_price_fetch());
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    msgY += 30;
    tft.setCursor(0, msgY);
    msg = "lwc: ";
    msg += timeFromTimestamp(stats.get_last_wifi_connect());
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    msgY += 30;
    tft.setCursor(0, msgY);
    msg = "lwd: ";
    msg += timeFromTimestamp(stats.get_last_wifi_disconnect());
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
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
    TRC_I_FUNC
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
