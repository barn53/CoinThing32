#include "gecko.h"
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

Gecko::Gecko()
{
}

bool Gecko::valid() const
{
    return !m_prices.empty();
}

void Gecko::newSettings()
{
    TraceFunction;
    {
        RecursiveMutexGuard(geckoSyncMutex);
        RecursiveMutexGuard(coinsMutex);
        m_settings = settings.coins();
        m_prices.clear();
        m_chart_data.clear();
        m_cancel.store(false);
    }
}

const std::vector<Gecko::CoinPrices>& Gecko::getCoinPrices() const
{
    return m_prices;
}
const std::map<String, std::vector<float>>& Gecko::getChartData() const
{
    return m_chart_data;
}
const GeckoSettings& Gecko::getSettings() const
{
    return m_settings;
}

void Gecko::fetchPrices()
{
    TraceFunction;
    String currency1(m_settings.currency1Lower());
    String currency2(m_settings.currency2Lower());
    String url(F("https://api.coingecko.com/api/v3/simple/price?include_24hr_change=true&include_market_cap=true&include_24hr_vol=true&vs_currencies="));
    url += currency1;
    url += F(",");
    url += currency2;
    url += F("&ids=");
    for (const auto& coin : m_settings.coins()) {
        url += coin.id;
        url += F(",");
    }

    TraceIPrintln(url);
    DynamicJsonDocument doc(2048);
    bool redo(true);
    do {
        if (httpJson.read(url.c_str(), doc)) {
            if (m_cancel.load()) {
                TraceIPrintln("Gecko::fetchPrices() cancelled");
                return;
            }
            RecursiveMutexGuard(geckoSyncMutex);
            m_prices.clear();
            for (const auto& coin : m_settings.coins()) {
                CoinPrices coinValues;
                coinValues.priceCurrency1 = doc[coin.id][currency1] | std::numeric_limits<float>::infinity();
                coinValues.priceCurrency2 = doc[coin.id][currency2] | std::numeric_limits<float>::infinity();
                coinValues.change24hCurrency1 = doc[coin.id][currency1 + "_24h_change"] | std::numeric_limits<float>::infinity();
                coinValues.change24hCurrency2 = doc[coin.id][currency2 + "_24h_change"] | std::numeric_limits<float>::infinity();
                coinValues.volume24hCurrency1 = doc[coin.id][currency1 + "_24h_vol"] | std::numeric_limits<float>::infinity();
                coinValues.volume24hCurrency2 = doc[coin.id][currency2 + "_24h_vol"] | std::numeric_limits<float>::infinity();
                coinValues.marketCapCurrency1 = doc[coin.id][currency1 + "_market_cap"] | std::numeric_limits<float>::infinity();
                coinValues.marketCapCurrency2 = doc[coin.id][currency2 + "_market_cap"] | std::numeric_limits<float>::infinity();
                m_prices.emplace_back(coinValues);
            }
            redo = false;
            stats.inc_gecko_price_fetch();
            esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdAllPricesUpdated, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
        } else {
            stats.inc_gecko_price_fetch_fail();
            TraceIPrintln("HTTP read failed!");
            if (WiFi.status() != WL_CONNECTED) {
                redo = false;
            }
        }
    } while (redo);
}

void Gecko::fetchCharts()
{
    TraceFunction;
    DynamicJsonDocument filter(16);
    filter["prices"] = true;
    DynamicJsonDocument doc(12288);
    String currency1(m_settings.currency1Lower());
    size_t successful(0);
    for (const auto& coin : m_settings.coins()) {
        String url(F("https://api.coingecko.com/api/v3/coins/"));
        url += coin.id;
        url += F("/market_chart?vs_currency=");
        url += currency1;
        url += F("&days=24&interval=daily");
        TraceIPrintln(url);

        bool redo(true);
        do {
            if (httpJson.read(url.c_str(), doc)) {
                if (m_cancel.load()) {
                    TraceIPrintln("Gecko::fetchCharts() cancelled");
                    return;
                }
                RecursiveMutexGuard(geckoSyncMutex);
                JsonArray jPrices(doc["prices"]);
                auto& coinChartData(m_chart_data[coin.id]);
                coinChartData.clear();
                coinChartData.reserve(jPrices.size());
                for (const auto& p : jPrices) {
                    coinChartData.emplace_back(p[1].as<float>());
                }
                redo = false;
                stats.inc_gecko_chart_fetch();
                ++successful;
                esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdChartUpdated, (void*)coin.id.c_str(), coin.id.length() + 1, 0);
            } else {
                stats.inc_gecko_chart_fetch_fail();
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

    if (successful == m_settings.coins().size()) {
        esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdAllChartsUpdated, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
    }
}

String Gecko::toJson() const
{
    TraceFunction;
    RecursiveMutexGuard(geckoSyncMutex);
    String json("{");

    json += R"("coins":[)";
    size_t counter(0);
    for (const auto& coin : m_settings.coins()) {
        if (counter > 0) {
            json += ",";
        }
        json += R"({"name":")" + coin.name + R"(")";
        json += R"(,"symbol":")" + coin.symbol + R"(")";
        json += R"(,"id":")" + coin.id + R"(")";

        json += R"(,"price )" + m_settings.getCurrencies()[0].symbol + R"(":)" + m_prices[counter].priceCurrency1;
        json += R"(,"24h change )" + m_settings.getCurrencies()[0].symbol + R"(":")" + String(m_prices[counter].change24hCurrency1) + R"(%")";
        json += R"(,"market capitalization )" + m_settings.getCurrencies()[0].symbol + R"(":)" + m_prices[counter].marketCapCurrency1;
        json += R"(,"24h volume )" + m_settings.getCurrencies()[0].symbol + R"(":)" + m_prices[counter].volume24hCurrency1;

        json += R"(,"price )" + m_settings.getCurrencies()[1].symbol + R"(":)" + m_prices[counter].priceCurrency2;
        json += R"(,"24h change )" + m_settings.getCurrencies()[1].symbol + R"(":")" + String(m_prices[counter].change24hCurrency2) + R"(%")";
        json += R"(,"market capitalization )" + m_settings.getCurrencies()[1].symbol + R"(":)" + m_prices[counter].marketCapCurrency2;
        json += R"(,"24h volume )" + m_settings.getCurrencies()[1].symbol + R"(":)" + m_prices[counter].volume24hCurrency2;

        json += "}";
        ++counter;
    }
    json += ("]"); // coins

    json += R"(,"charts":[)";
    counter = 0;
    for (const auto& cd : m_chart_data) {
        if (counter > 0) {
            json += ",";
        }
        json += R"({"id":")" + cd.first + R"(")";
        json += R"(,"data":[)";
        size_t counter2(0);
        for (const auto& d : cd.second) {
            if (counter2 > 0) {
                json += ",";
            }
            json += String(d);
            ++counter2;
        }
        json += "]}";
        ++counter;
    }
    json += ("]"); // charts

    json += ("}"); // top level
    return json;
}

Gecko gecko;
SemaphoreHandle_t geckoSyncMutex = xSemaphoreCreateRecursiveMutex();
QueueHandle_t geckoQueue = xQueueCreate(5, sizeof(GeckoRemit));
TaskHandle_t geckoTaskHandle;

void geckoTask(void*)
{
    GeckoRemit type;

    while (true) {
        if (xQueueReceive(geckoQueue, reinterpret_cast<void*>(&type), portMAX_DELAY)) {
            TraceNIPrintf("Notification type: %u\n", static_cast<uint32_t>(type));
            switch (type) {
            case GeckoRemit::settingsChanged:
                gecko.newSettings();
                gecko.fetchPrices();
                gecko.fetchCharts();
                break;
            case GeckoRemit::fetchPrices:
                gecko.fetchPrices();
                break;
            case GeckoRemit::fetchCharts:
                gecko.fetchCharts();
                break;
            }
        }
    }
}

void createGeckoTask()
{
    TraceFunction;
    xTaskCreatePinnedToCore(
        geckoTask, /* Task function. */
        "geckoTask", /* name of task. */
        TASK_STACK_SIZE, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &geckoTaskHandle, /* Task handle to keep track of created task */
        0);
}

} // namespace cointhing
