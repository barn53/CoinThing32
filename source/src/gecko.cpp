#include "gecko.h"
#include "events.h"
#include "http_json.h"
#include "semaphores.h"
#include "utils.h"

#include <ArduinoJson.h>

namespace cointhing {

Gecko::Gecko()
{
}

void Gecko::set(const Settings& settings)
{
    TRC_I_FUNC
    RecursiveMutexGuard guard(geckoDataMutex);
    m_settings = settings;
}

const CoinData& Gecko::getData(size_t index) const
{
    TRC_I_FUNC
    if (m_settings.coins().size() < index) {
        const auto data = m_coin_data.find(m_settings.coins()[index].id);
        if (data != m_coin_data.end()) {
            return data->second;
        }
    }
    return CoinData();
}

const SettingsCoins& Gecko::getSettings() const
{
    TRC_I_FUNC
    return m_settings;
}

void Gecko::fetchAll()
{
    TRC_I_FUNC
    {
        RecursiveMutexGuard guard(geckoDataMutex);
        m_coin_data.clear();

        for (const auto& coin : m_settings.coins()) {
            fetchOne(coin);
        }
    }
    esp_event_post_to(loopHandle, ESP_EVENT_COINTHING_BASE, eventIdDisplay, (void*)__PRETTY_FUNCTION__, strlen(__PRETTY_FUNCTION__) + 1, 0);
}

void Gecko::fetchOne(const SettingsCoins::Coin& coin)
{
    TRC_I_FUNC
    RecursiveMutexGuard guard(geckoDataMutex);

    DynamicJsonDocument doc(4096);
    String lowerCurrency(m_settings.currency1());
    lowerCurrency.toLowerCase();
    String url(F("https://api.coingecko.com/api/v3/simple/price?"));
    url += F("vs_currencies=");
    url += lowerCurrency;
    url += F("&ids=");
    url += coin.id;
    url += F("&include_24hr_change=true");
    if (httpJson.read(url.c_str(), doc)) {
        float current_price = doc[coin.id][lowerCurrency] | std::numeric_limits<float>::infinity();
        TRC_I_PRINTF("coin: %s\n", coin.name.c_str());
        TRC_I_PRINTF("current_price: %f\n", current_price);
    } else {
        TRC_I_PRINTLN("HTTP read failed!");
    }
}

void Gecko::clear()
{
    RecursiveMutexGuard guard(geckoDataMutex);
    m_settings.clear();
    m_coin_data.clear();
}

SemaphoreHandle_t geckoDataMutex(xSemaphoreCreateRecursiveMutex());
Gecko gecko;
TaskHandle_t geckoTaskHandle;

void geckoTask(void*)
{
    while (true) {
        if (xSemaphoreTake(fetchSemaphore, portMAX_DELAY)) {
            gecko.fetchAll();
        }
    }
}

void createGeckoTask()
{
    xTaskCreate(
        geckoTask, /* Task function. */
        "geckoTask", /* name of task. */
        10000, /* Stack size of task */
        nullptr, /* parameter of the task */
        0, /* priority of the task */
        &geckoTaskHandle /* Task handle to keep track of created task */);
}

} // namespace cointhing
