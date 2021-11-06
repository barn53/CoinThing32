#include "gecko.h"
#include "http_json.h"
#include "semaphores.h"
#include "utils.h"

#include <ArduinoJson.h>

namespace cointhing {

Gecko::Gecko()
{
    m_mutex = xSemaphoreCreateRecursiveMutex();
}

void Gecko::set(const Settings& settings)
{
    TRC_I_FUNC
    RecursiveMutexGuard guard(m_mutex);
    m_settings = settings;
}

void Gecko::fetchAll()
{
    TRC_I_FUNC
    RecursiveMutexGuard guard(m_mutex);

    for (const auto& coin : m_settings.coins()) {
        fetchOne(coin);
    }
}

void Gecko::fetchOne(const SettingsCoins::Coin& coin)
{
    RecursiveMutexGuard guard(m_mutex);

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
    RecursiveMutexGuard guard(m_mutex);
    m_settings.clear();
    m_coin_data.clear();
}

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
