#include "gecko.h"
#include "events.h"
#include "http_json.h"
#include "semaphores.h"
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

void Gecko::fetchPrices()
{
    TRC_I_FUNC
    SettingsCoins tempCoins;
    {
        RecursiveMutexGuard guard(coinsMutex);
        tempCoins = settings.coins();
    }

    String currency1(tempCoins.currency1Lower());
    String currency2(tempCoins.currency2Lower());
    String url(F("https://api.coingecko.com/api/v3/simple/price?include_24hr_change=true&include_market_cap=true&include_24hr_vol=true&vs_currencies="));
    url += currency1;
    url += F(",");
    url += currency2;
    url += F("&ids=");
    for (const auto& coin : tempCoins.coins()) {
        url += coin.id;
        url += F(",");
    }

    TRC_I_PRINTLN(url);
    DynamicJsonDocument doc(2048);
    std::vector<CoinPrices> tempValues;

    if (httpJson.read(url.c_str(), doc)) {
        for (const auto& coin : tempCoins.coins()) {
            CoinPrices coinValues;
            coinValues.priceCurrency1 = doc[coin.id][currency1] | std::numeric_limits<float>::infinity();
            coinValues.priceCurrency2 = doc[coin.id][currency2] | std::numeric_limits<float>::infinity();
            coinValues.change24hCurrency1 = doc[coin.id][currency1 + "_24h_change"] | std::numeric_limits<float>::infinity();
            coinValues.change24hCurrency2 = doc[coin.id][currency2 + "_24h_change"] | std::numeric_limits<float>::infinity();
            coinValues.volume24hCurrency1 = doc[coin.id][currency1 + "_24h_vol"] | std::numeric_limits<float>::infinity();
            coinValues.volume24hCurrency2 = doc[coin.id][currency2 + "_24h_vol"] | std::numeric_limits<float>::infinity();
            coinValues.marketCapCurrency1 = doc[coin.id][currency1 + "_market_cap"] | std::numeric_limits<float>::infinity();
            coinValues.marketCapCurrency2 = doc[coin.id][currency2 + "_market_cap"] | std::numeric_limits<float>::infinity();
            tempValues.emplace_back(coinValues);
        }

        {
            RecursiveMutexGuard guard(coinsMutex);
            std::swap(m_settings, tempCoins);
            m_prices.swap(tempValues);
            esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdAllPricesUpdated, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
        }
    } else {
        TRC_I_PRINTLN("HTTP read failed!");
    }
}

void Gecko::fetchCharts()
{
    TRC_I_FUNC
    SettingsCoins tempCoins;
    {
        RecursiveMutexGuard guard(coinsMutex);
        tempCoins = settings.coins();

        // All coin ids we currently have chart data for
        std::set<String> chartIds;
        for (const auto& cd : m_chart_data) {
            chartIds.emplace(cd.first);
        }
        // All coin ids in the settings
        std::set<String> tempIds;
        for (const auto& c : tempCoins.coins()) {
            tempIds.emplace(c.id);
        }
        std::vector<String> removeChart;
        std::set_difference(chartIds.begin(), chartIds.end(), tempIds.begin(), tempIds.end(), std::back_inserter(removeChart));
        for (const auto& r : removeChart) {
            m_chart_data.erase(r);
        }
    }

    DynamicJsonDocument filter(16);
    filter["prices"] = true;
    DynamicJsonDocument doc(12288);

    String currency1(tempCoins.currency1Lower());
    for (const auto& coin : tempCoins.coins()) {
        String url(F("https://api.coingecko.com/api/v3/coins/"));
        url += coin.id;
        url += F("/market_chart?vs_currency=");
        url += currency1;
        url += F("&days=24&interval=daily");
        TRC_I_PRINTLN(url);

        if (httpJson.read(url.c_str(), doc)) {
            JsonArray jPrices(doc["prices"]);
            RecursiveMutexGuard guard(coinsMutex);
            auto& chartData(m_chart_data[coin.id]);
            chartData.clear();
            chartData.reserve(jPrices.size());
            for (const auto& p : jPrices) {
                chartData.emplace_back(p[1].as<float>());
            }
            esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdChartUpdated, (void*)coin.id.c_str(), coin.id.length() + 1, 0);
        } else {
            TRC_I_PRINTLN("HTTP read failed!");
        }
    }
    esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdAllChartsUpdated, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
}

Gecko gecko;
TaskHandle_t geckoPriceTaskHandle;
TaskHandle_t geckoChartTaskHandle;

void geckoPriceTask(void*)
{
    while (true) {
        if (xSemaphoreTake(fetchPriceSemaphore, portMAX_DELAY)) {
            gecko.fetchPrices();
        }
    }
}

void geckoChartTask(void*)
{
    while (true) {
        if (xSemaphoreTake(fetchChartSemaphore, portMAX_DELAY)) {
            gecko.fetchCharts();
        }
    }
}

void createGeckoTasks()
{
    xTaskCreatePinnedToCore(
        geckoPriceTask, /* Task function. */
        "geckoPriceTask", /* name of task. */
        TASK_STACK_SIZE, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &geckoPriceTaskHandle, /* Task handle to keep track of created task */
        0);

    xTaskCreatePinnedToCore(
        geckoChartTask, /* Task function. */
        "geckoChartTask", /* name of task. */
        TASK_STACK_SIZE, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &geckoChartTaskHandle, /* Task handle to keep track of created task */
        0);
}

} // namespace cointhing
