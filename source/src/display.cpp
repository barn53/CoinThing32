#include "display.h"
#include "events.h"
#include "finnhub.h"
#include "gecko.h"
#include "main.h"
#include "settings.h"
#include "stats.h"
#include "tasks.h"

#include <ArduinoJson.h>

namespace cointhing {

SemaphoreHandle_t Display::m_tft_sync_mutex = xSemaphoreCreateRecursiveMutex();

Display::Display()
{
    resetIds();
}

void Display::begin() const
{
    TraceFunction;
    RecursiveMutexGuard(m_tft_sync_mutex);
    tft.begin();
    tft.setRotation(0); // 0 & 2 Portrait. 1 & 3 landscape
    tft.setTextWrap(false);

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.loadFont(F("NotoSans-Regular20"));
    String msg("ConThing 2\"");
    tft.setCursor((TFT_WIDTH - tft.textWidth(msg)) / 2, 100);
    tft.print(msg);
    tft.unloadFont();
}

void Display::resetIds()
{
    m_display_category = DisplayCategory::None;
    m_display_gecko_index = 0;
    m_display_finnhub_index = 0;

    if (!gecko.getCoinPrices().empty()) {
        m_display_category = DisplayCategory::Gecko;
    } else if (!finnhub.getPrices().empty()) {
        m_display_category = DisplayCategory::Gecko;
    }
}

void Display::nextId()
{
    if (m_display_category == DisplayCategory::Gecko) {
        ++m_display_gecko_index;
        if (m_display_gecko_index >= gecko.getCoinPrices().size()) {
            m_display_gecko_index = 0;
            if (!finnhub.getPrices().empty()) {
                m_display_category = DisplayCategory::Finnhub;
                m_display_finnhub_index = 0;
            }
        }
    } else if (m_display_category == DisplayCategory::Finnhub) {
        ++m_display_finnhub_index;
        if (m_display_finnhub_index >= finnhub.getPrices().size()) {
            m_display_finnhub_index = 0;
            if (!gecko.getCoinPrices().empty()) {
                m_display_category = DisplayCategory::Gecko;
                m_display_gecko_index = 0;
            }
        }
    } else {
        resetIds();
    }
}

void Display::show() const
{
    TraceFunction;

    if (m_display_category == DisplayCategory::Gecko) {
        showGecko();
    } else if (m_display_category == DisplayCategory::Finnhub) {
        showFinnhub();
    } else {
        showNothing();
    }
}

void Display::showGecko() const
{
    TraceFunction;
    TraceIPrintf("Show coin ID: %u\n", m_display_gecko_index);

    RecursiveMutexGuard(m_tft_sync_mutex);
    if (m_last_shown != DisplayShown::Gecko) {
        tft.fillScreen(TFT_BLACK);
    }

    for (const auto& c : gecko.getChartData()) {
        // TraceIPrintf("Coin: %s - values: %u\n", c.first.c_str(), c.second.size());
    }

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.loadFont(F("NotoSans-Regular20"));

    const auto& geckoSettings(gecko.getSettings());
    const auto& coinPrices(gecko.getCoinPrices()[m_display_gecko_index]);
    String msg(geckoSettings.name(m_display_gecko_index));

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
    msg = "crashes: ";
    msg += String(stats.get_crash_counter());
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_BLACK);
    tft.print(msg);

    tft.unloadFont();
    m_last_shown = DisplayShown::Gecko;
}

void Display::showFinnhub() const
{
    TraceFunction;
    TraceIPrintf("Show Finnhub ID: %u\n", m_display_finnhub_index);

    RecursiveMutexGuard(m_tft_sync_mutex);
    if (m_last_shown != DisplayShown::Finnhub) {
        tft.fillScreen(TFT_WHITE);
    }

    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.loadFont(F("NotoSans-Regular20"));

    const auto& finnhubSettings(finnhub.getSettings());
    const auto& stockPrices(finnhub.getPrices()[m_display_finnhub_index]);
    String msg(finnhubSettings.symbol(m_display_finnhub_index));

    int16_t msgY(10);
    tft.setCursor(0, msgY);
    msg += ": ";
    msg += stockPrices.current;
    msg += "$";
    tft.fillRect(tft.textWidth(msg) - 5, msgY, TFT_WIDTH - (tft.textWidth(msg) - 5), 20, TFT_WHITE);
    tft.print(msg);

    m_last_shown = DisplayShown::Finnhub;
}

void Display::showNothing() const
{
    TraceFunction;
    if (m_last_shown != DisplayShown::Nothing) {
        RecursiveMutexGuard(m_tft_sync_mutex);
        tft.fillScreen(TFT_BLACK);
        tft.loadFont(F("NotoSans-Regular20"));
        tft.fillScreen(TFT_DARKGREEN);
        tft.setTextColor(TFT_WHITE, TFT_DARKGREEN);
        tft.setCursor(10, 100);
        String msg("Nothing to see");
        tft.print(msg);
        tft.unloadFont();
        m_last_shown = DisplayShown::Nothing;
    }
}

void Display::showNewSettings() const
{
    TraceFunction;
    RecursiveMutexGuard(m_tft_sync_mutex);

    tft.loadFont(F("NotoSans-Regular20"));
    tft.fillScreen(TFT_GOLD);
    tft.setTextColor(TFT_WHITE, TFT_GOLD);
    tft.setCursor(10, 100);
    String msg("New Settings");
    tft.print(msg);
    tft.unloadFont();
    vTaskDelay(4000);
    m_last_shown = DisplayShown::NewSettings;
}

TFT_eSPI tft;
Display display;
TaskHandle_t displayTaskHandle;

void displayTask(void*)
{
    DisplayNotificationType notificationType;
    while (true) {
        if (xTaskNotifyWait(0, 0xffffffff, reinterpret_cast<uint32_t*>(&notificationType), portMAX_DELAY)) {
            TraceNIPrintf("Notification type: %u\n", static_cast<uint32_t>(notificationType));
            switch (notificationType) {
            case DisplayNotificationType::settingsChanged:
                display.resetIds();
                display.showNewSettings();
                break;
            case DisplayNotificationType::showNextId:
                RecursiveMutexGuard(geckoSyncMutex);
                RecursiveMutexGuard(finnhubSyncMutex);
                display.show();
                display.nextId();
                break;
            }
        }
    }
}

void createDisplayTask()
{
    TraceFunction;
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
