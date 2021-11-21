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
    TRC_I_FUNC
    {
        RecursiveMutexGuard g1(geckoSyncMutex);
        RecursiveMutexGuard g2(coinsMutex);
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
    TRC_I_FUNC
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

    TRC_I_PRINTLN(url);
    DynamicJsonDocument doc(2048);
    bool redo(true);
    do {
        if (httpJson.read(url.c_str(), doc)) {
            if (m_cancel.load()) {
                TRC_I_PRINTLN("Gecko::fetchPrices() cancelled");
                return;
            }
            RecursiveMutexGuard g(geckoSyncMutex);
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
        } else {
            stats.inc_gecko_price_fetch_fail();
            TRC_I_PRINTLN("HTTP read failed!");
        }
    } while (redo);

    esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdAllPricesUpdated, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
}

void Gecko::fetchCharts()
{
    TRC_I_FUNC
    DynamicJsonDocument filter(16);
    filter["prices"] = true;
    DynamicJsonDocument doc(12288);
    String currency1(m_settings.currency1Lower());
    for (const auto& coin : m_settings.coins()) {
        String url(F("https://api.coingecko.com/api/v3/coins/"));
        url += coin.id;
        url += F("/market_chart?vs_currency=");
        url += currency1;
        url += F("&days=24&interval=daily");
        TRC_I_PRINTLN(url);

        bool redo(true);
        do {
            if (httpJson.read(url.c_str(), doc)) {
                if (m_cancel.load()) {
                    TRC_I_PRINTLN("Gecko::fetchCharts() cancelled");
                    return;
                }
                RecursiveMutexGuard g(geckoSyncMutex);
                JsonArray jPrices(doc["prices"]);
                auto& coinChartData(m_chart_data[coin.id]);
                coinChartData.clear();
                coinChartData.reserve(jPrices.size());
                for (const auto& p : jPrices) {
                    coinChartData.emplace_back(p[1].as<float>());
                }
                redo = false;
                stats.inc_gecko_chart_fetch();
                esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdChartUpdated, (void*)coin.id.c_str(), coin.id.length() + 1, 0);
            } else {
                stats.inc_gecko_chart_fetch_fail();
                TRC_I_PRINTLN("HTTP read failed!");
            }
        } while (redo);
    }

    esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdAllChartsUpdated, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
}

String Gecko::toJson() const
{
    TRC_I_FUNC
    RecursiveMutexGuard g(geckoSyncMutex);
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
TaskHandle_t geckoTaskHandle;

void geckoTask(void*)
{
    GeckoNotificationType notificationType;

    while (true) {
        if (xTaskNotifyWait(0, 0xffffffff, reinterpret_cast<uint32_t*>(&notificationType), portMAX_DELAY)) {
            TRC_I_PRINTF("Notification type: %u\n", static_cast<uint32_t>(notificationType));
            switch (notificationType) {
            case GeckoNotificationType::settingsChanged:
                gecko.newSettings();
                gecko.fetchPrices();
                gecko.fetchCharts();
                break;
            case GeckoNotificationType::fetchPrices:
                gecko.fetchPrices();
                break;
            case GeckoNotificationType::fetchCharts:
                gecko.fetchCharts();
                break;
            }
        }
    }
}

void createGeckoTask()
{
    TRC_I_FUNC
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
