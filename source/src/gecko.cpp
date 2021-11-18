#include "gecko.h"
#include "events.h"
#include "http_json.h"
#include "main.h"
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
        m_settings_coins = settings.coins();
        m_prices.clear();
        m_chart_data.clear();
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
const SettingsCoins& Gecko::getSettingsCoins() const
{
    return m_settings_coins;
}

void Gecko::fetchPrices()
{
    TRC_I_FUNC
    String currency1(m_settings_coins.currency1Lower());
    String currency2(m_settings_coins.currency2Lower());
    String url(F("https://api.coingecko.com/api/v3/simple/price?include_24hr_change=true&include_market_cap=true&include_24hr_vol=true&vs_currencies="));
    url += currency1;
    url += F(",");
    url += currency2;
    url += F("&ids=");
    for (const auto& coin : m_settings_coins.coins()) {
        url += coin.id;
        url += F(",");
    }

    TRC_I_PRINTLN(url);
    DynamicJsonDocument doc(2048);
    if (httpJson.read(url.c_str(), doc)) {
        RecursiveMutexGuard g(geckoSyncMutex);
        for (const auto& coin : m_settings_coins.coins()) {
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

        ++m_count_price_fetches;
        esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdAllPricesUpdated, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
    } else {
        TRC_I_PRINTLN("HTTP read failed!");
    }
}

void Gecko::fetchCharts()
{
    TRC_I_FUNC
    m_cancel.store(false);
    DynamicJsonDocument filter(16);
    filter["prices"] = true;
    DynamicJsonDocument doc(12288);
    String currency1(m_settings_coins.currency1Lower());
    for (const auto& coin : m_settings_coins.coins()) {
        String url(F("https://api.coingecko.com/api/v3/coins/"));
        url += coin.id;
        url += F("/market_chart?vs_currency=");
        url += currency1;
        url += F("&days=24&interval=daily");
        TRC_I_PRINTLN(url);

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
            ++m_count_chart_fetches;
            esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdChartUpdated, (void*)coin.id.c_str(), coin.id.length() + 1, 0);
        } else {
            TRC_I_PRINTLN("HTTP read failed!");
        }
    }
    esp_event_post_to(loopHandle, COINTHING_EVENT_BASE, eventIdAllChartsUpdated, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
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
