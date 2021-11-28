#include "finnhub.h"
#include "events.h"
#include "http_json.h"
#include "main.h"
#include "stats.h"
#include "tasks.h"
#include "utils.h"

#include <ArduinoJson.h>
#include <algorithm>
#include <set>

namespace cointhing {

Finnhub::Finnhub()
{
}

bool Finnhub::valid() const
{
    return !m_prices.empty();
}

void Finnhub::newSettings()
{
    TraceFunction;
    {
        RecursiveMutexGuard(finnhubSyncMutex);
        RecursiveMutexGuard(settingsMutex);
        m_settings = settings.finnhub();
        m_prices.clear();
        m_chart_data.clear();
        unCancel();
    }
}

void Finnhub::cancel() const
{
    TraceFunction;
    m_cancel = true;
}

void Finnhub::unCancel() const
{
    TraceFunction;
    m_cancel = false;
}

const std::vector<Finnhub::StockPrices>& Finnhub::getPrices() const
{
    return m_prices;
}
const std::map<String, std::vector<float>>& Finnhub::getChartData() const
{
    return m_chart_data;
}
const FinnhubSettings& Finnhub::getSettings() const
{
    return m_settings;
}

void Finnhub::fetchPrices()
{
    TraceFunction;

    DynamicJsonDocument doc(192);
    size_t successful(0);
    std::vector<StockPrices> tempPrices;
    for (const auto& symbol : m_settings.symbols()) {
        String url(F("https://finnhub.io/api/v1/quote?symbol="));
        url += symbol;
        url += "&token=";
        url += m_settings.apiToken();
        TraceIPrintln(url);
        bool redo(true);

        do {
            if (httpJson.read(url.c_str(), doc)) {
                if (m_cancel) {
                    TraceIPrintln("fetch loop cancelled");
                    return;
                }

                StockPrices sp;
                sp.current = doc["c"];
                sp.change = doc["d"];
                sp.percentChange = doc["dp"];
                sp.highDay = doc["h"];
                sp.lowDay = doc["l"];
                sp.openDay = doc["o"];
                sp.closePrevious = doc["pc"];
                sp.timestamp = doc["t"];
                tempPrices.emplace_back(sp);

                redo = false;
                stats.inc_finnhub_price_fetch();
                ++successful;
            } else {
                stats.inc_finnhub_price_fetch_fail();
                TraceIPrintln("HTTP read failed!");
                if (WiFi.status() != WL_CONNECTED) {
                    redo = false;
                }
            }
        } while (redo);

        if (WiFi.status() != WL_CONNECTED) {
            break;
        }
    }

    if (successful == m_settings.symbols().size()) {
        RecursiveMutexGuard(finnhubSyncMutex);
        m_prices.swap(tempPrices);
        esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdAllFinnhubPricesUpdated, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
    }
}

void Finnhub::fetchCharts()
{
    TraceFunction;
    // todo
}

String Finnhub::toJson() const
{
    TraceFunction;
    RecursiveMutexGuard(finnhubSyncMutex);
    String json(R"("finnhub":{)");

    json += R"("prices":[)";
    size_t counter(0);
    for (const auto& symbol : m_settings.symbols()) {
        if (counter > 0) {
            json += ",";
        }
        json += R"({"symbol":")" + symbol + R"(")";
        if (m_prices.size() > counter) {
            const auto prices(m_prices[counter]);
            json += R"(,"current price $":)" + String(prices.current);
            json += R"(,"change $":)" + String(prices.change);
            json += R"(,"change $ %":)" + String(prices.percentChange);
            json += R"(,"high price of day $":)" + String(prices.highDay);
            json += R"(,"low price of day $":)" + String(prices.lowDay);
            json += R"(,"open price of day $":)" + String(prices.openDay);
            json += R"(,"previous close price $":)" + String(prices.closePrevious);
            json += R"(,"time":")" + timeFromTimestamp(prices.timestamp) + R"(")";
        }
        json += "}"; // symbol
        ++counter;
    }
    json += ("]"); // stocks

    json += R"(,"charts":[)";
    json += ("]"); // charts

    json += ("}"); // finnhub
    return json;
}

Finnhub finnhub;
SemaphoreHandle_t finnhubSyncMutex = xSemaphoreCreateRecursiveMutex();
QueueHandle_t finnhubQueue = xQueueCreate(5, sizeof(FinnhubRemit));
TaskHandle_t finnhubTaskHandle;

void finnhubTask(void*)
{
    FinnhubRemit type;

    while (true) {
        if (xQueueReceive(finnhubQueue, reinterpret_cast<void*>(&type), portMAX_DELAY)) {
            TraceNIPrintf("Remit type: %u\n", static_cast<uint32_t>(type));
            switch (type) {
            case FinnhubRemit::settingsChanged:
                finnhub.newSettings();
                finnhub.fetchPrices();
                finnhub.fetchCharts();
                break;
            case FinnhubRemit::fetchPrices:
                finnhub.fetchPrices();
                break;
            case FinnhubRemit::fetchCharts:
                finnhub.fetchCharts();
                break;
            }
            finnhub.unCancel();
        }
    }
}

void createFinnhubTask()
{
    TraceFunction;
    xTaskCreatePinnedToCore(
        finnhubTask, /* Task function. */
        "finnhubTask", /* name of task. */
        TASK_STACK_SIZE, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &finnhubTaskHandle, /* Task handle to keep track of created task */
        0);
}

} // namespace cointhing
